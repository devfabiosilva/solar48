#include <stddef.h>
#include <usb_device.h>
#include <gpios.h>
#include <time.h>
//#include <stdlib.h>
//dmesg -w
//sudo modprobe usbmon
//sudo cat /sys/kernel/debug/usb/usbmon/1u
void usb_error(int);
void setup()
{
  init_usb_device(usb_error);
  init_gpios();
}

void run(void)
{
  delay_test();
}

void halt()
{
// It would not happen
}

void usb_error(int value)
{

}
