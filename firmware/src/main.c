#include <stddef.h>
#include <system.h>
#include <usb_device.h>
#include <rtc.h>
#include <usb_io.h>
#include <gpios.h>
#include <time.h>
#include <errors.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

//#include <stdlib.h>
//dmesg -w
//sudo modprobe usbmon
//sudo cat /sys/kernel/debug/usb/usbmon/1u
// lsusb -d 0483:5740 -v | grep -iE 'manufacturer|product|serial'
volatile int hasError = 0;
void usb_receive(uint8_t *, uint32_t);
void usb_receive_complete();
void usb_error(int);
void realtime(uint32_t);

void setup()
{
  init_usb_device(usb_receive, usb_receive_complete, usb_error);
  init_rtc(NULL);
  init_gpios();

  END_SETUP
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

    //usb_print_memory_info();
    delay_seconds(2);
  }
}

void halt()
{
  // It could not happen. If happens report bug and disable all interrupts
  DISABLE_SETUP
}

void halt_ir()
{
  // It could not happen. If happens report bug and disable all interrupts
  DISABLE_SETUP
}

static char text[1024];
size_t text_sz = 0;

void usb_receive(uint8_t *buf, uint32_t buf_sz)
{
  if ((size_t)buf_sz > sizeof(text)-1)
    text_sz = sizeof(text)-1;
  else
    text_sz = (size_t)buf_sz;

  memcpy(text, buf, text_sz);
  text[text_sz] = 0;
  
}

#define COMMAND_CHECK(cmd, ...) \
  if (strncmp(text, cmd, sizeof(cmd)) == 0) { \
    usb_printf(__VA_ARGS__); \
    return; \
  }

#define COMMAND_CHECK_CALL(cmd, fn) \
  if (strncmp(text, cmd, sizeof(cmd)) == 0) {\
    fn(); \
    return; \
  }
 

//bool lock_loop = false;
void usb_receive_complete()
{
  if (text_sz <= 2)
    return;

  COMMAND_CHECK("ping", "\n\n\nPONG\n\n\n")
  COMMAND_CHECK_CALL("meminfo", usb_print_memory_info)
  COMMAND_CHECK("timestamp", "\n\n\nTIMESTAMP: %ld\n\n\n", rtc_get_timestamp())

  usb_printf("Invalid command %.*s\n\n", text_sz, text);

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

//TODO refactor. Testing
volatile int blink = 0;
void realtime(uint32_t time)
{

  usb_printf("\n\nTIMESTAMP: %ld | TEXT SIZE: %ld\n\n", rtc_get_timestamp(), text_sz);
  if (blink) {
    ledon();
    blink = 0;
  } else {
    ledoff();
    blink = 1;
  }
}

