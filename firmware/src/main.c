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
#include <console.h>

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
SOLAR48_DATE sd;

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

  text[text_sz] = 0;
  char *p = text;
  size_t tmp = text_sz;

  while (tmp > 0) {

    char c = *(buf++);
    if ((c != '\r') && (c != '\n'))
      *(p++) = c;
    else {
      *p = 0;
      break;
    }
 
    --tmp;
  }
}


void usb_receive_complete()
{
/*
  if (text_sz <= 2)
    return;
*/
  COMMAND_CHECK_CALL_ARG(ping)
  COMMAND_CHECK_CALL_ARG(meminfo)
  COMMAND_CHECK_CALL_ARG(timestamp)
  COMMAND_CHECK_CALL_ARG(setdate)
  COMMAND_CHECK_CALL_ARG(getdate)
  COMMAND_CHECK_CALL_ARG(help)

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

//  usb_printf("\n\nTIMESTAMP: %ld | TEXT SIZE: %ld\n\n", rtc_get_timestamp(), text_sz);
  uint32_t tm = time;
  get_solar48_date(&sd, &tm);

  usb_printf("\n\nTIME: %u:%u:%u\n\n", sd.hour, (uint32_t)sd.minute, (uint32_t)sd.second);
  usb_printf("\n\nDay (yyyy/mm/dd): %s - %u/%u/%u\n\n", get_day_str((int)sd.year, (int)sd.month, (int)sd.day), sd.year, (uint32_t)sd.month, (uint32_t)sd.day);

  if (blink) {
    ledon();
    blink = 0;
  } else {
    ledoff();
    blink = 1;
  }
}

