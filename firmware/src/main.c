#include <stddef.h>
#include <usb_device.h>
#include <usb_io.h>
#include <gpios.h>
#include <time.h>
#include <errors.h>

//#include <stdlib.h>
//dmesg -w
//sudo modprobe usbmon
//sudo cat /sys/kernel/debug/usb/usbmon/1u
// lsusb -d 0483:5740 -v | grep -iE 'manufacturer|product|serial'
volatile int hasError = 0;
void usb_error(int);

void setup()
{
  init_usb_device(usb_error);
  init_gpios();
}

void run(void)
{
  blink_n(4);
  while (1) {
    //blink_n(2);
    while (hasError) {
	blink_n(hasError);
	delay_seconds(1);
	hasError = 0;
    }

    usb_print_memory_usage();

  }

}

void halt()
{
// It would not happen
}

void usb_error(int value)
{

  switch(value) {
    case E_USB_INIT:
      hasError = 7;
      break;
    case E_USB_REGISTER_CLASS:
      hasError = 6;
      break;
    case E_USB_REGISTER_INTERFACE:
      hasError = 4;
      break;
    case E_USB_START:
      hasError = 3;
      break;
    case E_USB_TRANSMIT_BUSY:
      hasError =1;
      break;
    case E_USB_TRANSMIT_FAIL:
      hasError = 2;
      break;
    default:
      hasError = 5;
  }

}

