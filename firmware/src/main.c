#include <stddef.h>
#include <usb_device.h>
#include <usb_io.h>
#include <gpios.h>
#include <time.h>
#include <errors.h>
#include <stdio.h>
#include <string.h>

//#include <stdlib.h>
//dmesg -w
//sudo modprobe usbmon
//sudo cat /sys/kernel/debug/usb/usbmon/1u
// lsusb -d 0483:5740 -v | grep -iE 'manufacturer|product|serial'
volatile int hasError = 0;
void usb_receive(uint8_t *, uint32_t);
void usb_receive_complete();
void usb_error(int);

void setup()
{
  init_usb_device(usb_receive, usb_receive_complete, usb_error);
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
    delay_seconds(1);

  }

}

void halt()
{
// It would not happen
}

static char text[1024];
size_t text_sz = 0;

void usb_receive(uint8_t *buf, uint32_t buf_sz)
{
  if ((size_t)buf_sz > sizeof(text))
    text_sz = sizeof(text);
  else
    text_sz = (size_t)buf_sz;

  memcpy(text, buf, text_sz);
  
}

void usb_receive_complete()
{
  if (text_sz >= 4 && memcmp(text, "ping", 4) == 0) {
     const char *msg = "pong\r\n";
     CDC_Transmit_FS((uint8_t*)msg, strlen(msg));
  }
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
    case E_USB_HAL_PCD_HS:
      hasError = 8;
      break;
    default:
      hasError = 5;
  }

}

