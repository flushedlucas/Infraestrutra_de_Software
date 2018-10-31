#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

//probe function
// called on device insertion if and only if no other driver has bear us to the punch
static int pen_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
  printk(KERN_INFO "[*] Pen Drive (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
  return 0; //return 0 indicates we will manage this device
}

//disconnect
static void pen_disconnect(struct usb_interface *interface)
{
  printk(KERN_INFO "[*] Pen drive removed\n");
}

//usb_device_id
static struct usb_device_id pen_table[] = {
  //046d:c077
  {USB_DEVICE(0x046d, 0xc077)}, //"lsusb" at command line
  {}
};
MODULE_DEVICE_TABLE (usb, pen_table);

//usb_driver
static struct usb_driver pen_driver =
{
  .name = "Stick-Driver",
  .id_table = pen_table, //usb_device_id
  .probe = pen_probe,
  .disconnect = pen_disconnect,
};

static int __init pen_init(void)
{
  int ret = -1;
  printk(KERN_INFO "[*] Constructor of driver");
  printk(KERN_INFO "\tRegistering Driver with Kernel");
  ret = usb_register(&pen_driver);
  printk(KERN_INFO "\tRegistration is complete");
  return ret;
}

static void __exit pen_exit(void)
{
  //deregister
  printk(KERN_INFO "[*] Destructor of driver");
  usb_deregister(&pen_driver);
  printk(KERN_INFO "\tUnregistration complete!");
}

module_init(pen_init);
module_exit(pen_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucas Silva");
