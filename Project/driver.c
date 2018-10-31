#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define MOUSE_BASE 0x300

static int mouse_users = 0;
static int mouse_dx = 0;
static int mouse_dy = 0;
static int mouse_event = 0;

static struct wait_queue *mouse_wait;
// static spinlock_t mouse_lock = __SPIN_LOCK_UNLOCKED; //Deprecated
static DEFINE_SPINLOCK(mouse_lock);

static struct miscdevice mouse =
{
  MOUSE_MINOR, "mouse", &mouse_fops
};

//usb_device_id
static struct usb_device_id mouse_table[] = {
  //046d:c077
  {USB_DEVICE(0x046d, 0xc077)}, //"lsusb" at command line
  {}
};

//usb_driver
static struct usb_driver mouse_driver =
{
  .name = "Mouse-Driver",
  .id_table = mouse_table, //usb_device_id
  .probe = mouse_probe,
  .disconnect = mouse_disconnect,
};

struct file_operations mouse_fops = {
  owner: THIS_MODULE, /*gerenciador automático de uso*/
  read: read_mouse, /*ler o mouse*/
  write: write_mouse,
  poll: poll_mouse,
  open: open_mouse, /*chamado quando abrir*/
  release: close_mouse, /*chamado quando fechar*/
};

static int open_mouse(struct inode *inode, struct file *file)
{
    if (mouse_users++)
      return 0;

    if(request_irq(mouse_intr, MOUSE_IRQ, 0, "mouse", NULL))
    {
      mouse_users --;
      return -EBUSY;
    }
    mouse_dx = 0;
    mouse_dy = 0;
    mouse_event = 0;
    mouse_buttons = 0;
  return 0;
}

static int mouse_close(struct inode *inode, struct file *file)
{
  if(--mouse_users)
    return 0;
  free_irq(MOUSE_IRQ, NULL);
  return 0;
}

static ssize_t write_mouse(struct file *file, const char *buffer, size_t count, loff_t *ppos)
{
  return -EINVAL;
}

static void mouse_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
  char delta_x;
  char delta_y;
  unsigned char new_buttons;

  delta_x = inb(MOUSE_BASE);
  delta_y = inb(MOUSE_BASE+1);
  new_buttons = inb(MOUSE_BASE+2);

  if(delta_x || delta_y || new_buttons != mouse_buttons)
  {
    spin_lock(&mouse_lock);
    mouse_event = 1;
    mouse_dx += delta_x;
    mouse_dy += delta_y;
    mouse_buttons = new_buttons;
    spin_unlock(&mouse_lock);

    wake_up_interruptible(&mouse_wait);
  }
}

static unsigned int mouse_poll(struct file *file, pool_table *wait)
{
  poll_wait(file, &mouse_wait, wait);
  if (mouse_event)
    return POLLIN | POLLRDNORM
  return 0;
}

static ssize_t mouse_read(struct file *file, char *buffer, size_t count, loff_t *pos)
{
  int dx, dy;
  unsigned char button;
  unsigned long flags;
  int n;

  if(count<3)
    return -EINVAL;

  //Esperando o evento
  while(!mouse_event)
  {
    if(file->f_flags&O_NDELAY)
      return _EAGAIN;
    interruptible_sleep_on(&mouse_wait);
    if(signal_pending(current))
      return -ERESTARTSYS;
  }

  //Capturando o evento

  spinlock_irqsave(&mouse_lock, flags);

  dx = mouse_dx;
  dy = mouse_dy;
  button = mouse_buttons;

  if(dx <= -127)
    dx = -127;
  if(dx >= 127)
    dx = 127;
  if(dy <= -127)
    dy = -127;
  if(dy >= 127)
    dy = 127;

  mouse_dx -= dx;
  mouse_dy -= dy;

  if(mouse_dx == 0 && mouse_dy == 0)
    mouse_event = 0;

  spin_unlock_irqrestore(&mouse_lock, flags);

  if(put_user(button|0x80, buffer))
    return -EFAULT;
  if(put_user((char)dx, buffer+1))
    return -EFAULT;
  if(put_user((char)dy, buffer+2))
    return -EFAULT;
  for(n=3; n < count; n++)
    if(put_user(0x00, buffer+n))
      return -EFAULT;

  return count;
}

//probe function
// called on device insertion if and only if no other driver has bear us to the punch
static int mouse_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
  printk(KERN_INFO "[*] Mouse Drive: Mouse (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
  return 0; //return 0 indicates we will manage this device
}

//disconnect
static void mouse_disconnect(struct usb_interface *interface)
{
  printk(KERN_INFO "[*] Mouse drive removed\n");
}

__init ourmouse_init(void)
{
  if(request_region(MOUSE_BASE, 3, "mouse")<0){
    printk(KERN_ERR "mouse: request_region failed.\n");
    return -ENODEV;
  }

  if (misc_register(&mouse) < 0){
    printk(KERN_ERR "mouse: cannot register misc device.\n");
    release_region(MOUSE_BASE, 3);
    return -EBUSY;
  }

  return 0;
}


#ifdef MODULE

static int __init mouse_init(void)
{
  printk(KERN_INFO "[*] Constructor of driver");
  printk(KERN_INFO "\tRegistering Driver with Kernel");
  if(mouse_init() < 0){
    return -ENODEV;
  }
  printk(KERN_INFO "\tRegistration is complete");
  return 0;
}

static void __exit mouse_exit(void)
{
  //deregister
  printk(KERN_INFO "[*] Destructor of driver");
  misc_deregister(&mouse);
  free_region(MOUSE_BASE, 3);
  printk(KERN_INFO "\tUnregistration complete!");
}
#endif

module_init(mouse_init);
module_exit(mouse_exit);

MODULE_DEVICE_TABLE (usb, mouse_table);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucas Silva");
