#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

//probe function
// called on device insertion if and only if no other driver has bear us to the punch
static int mouse_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
  printk(KERN_INFO "[*] Mouse Drive (%04X:%04X) plugged\n", id->idVendor, id->idProduct);
  return 0; //return 0 indicates we will manage this device
}

//disconnect
static void mouse_disconnect(struct usb_interface *interface)
{
  printk(KERN_INFO "[*] Mouse drive removed\n");
}

//usb_device_id
static struct usb_device_id mouse_table[] = {
  //046d:c077
  {USB_DEVICE(0x046d, 0xc077)}, //"lsusb" at command line
  {}
};
MODULE_DEVICE_TABLE (usb, mouse_table);

//usb_driver
static struct usb_driver mouse_driver =
{
  .name = "Mouse-Driver",
  .id_table = mouse_table, //usb_device_id
  .probe = mouse_probe,
  .disconnect = mouse_disconnect,
};

static int __init mouse_init(void)
{
  int ret = -1;
  printk(KERN_INFO "[*] Constructor of driver");
  printk(KERN_INFO "\tRegistering Driver with Kernel");
  ret = usb_register(&mouse_driver);
  printk(KERN_INFO "\tRegistration is complete");
  return ret;
}

static void __exit mouse_exit(void)
{
  //deregister
  printk(KERN_INFO "[*] Destructor of driver");
  usb_deregister(&mouse_driver);
  printk(KERN_INFO "\tUnregistration complete!");
}

module_init(mouse_init);
module_exit(mouse_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lucas Silva");
