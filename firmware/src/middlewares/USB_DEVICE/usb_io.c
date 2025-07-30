#include <stdio.h>
#include <usb_device.h>
#include <memory.h>
#include <stdarg.h>
#include <usbd_cdc_if.h>
#include <console.h>
#include <string.h>

/*
sudo usermod -aG dialout $USER
minicom -D /dev/ttyACM0 -b 115200
screen /dev/ttyACM0 115200
cat /dev/ttyACM0
watch -n 0.5 "ls /dev/ttyACM*"
*/

DETAILED_RAM dr;
DETAILED_FLASH df;

static char printf_buffer[APP_TX_DATA_SIZE];
#define PRINTF_BUF_MAX_LEN sizeof(printf_buffer)

static char text[APP_TX_DATA_SIZE];
size_t text_sz = 0;

// Return USBD_OK if transmit was success, else error
int usb_printf(const char *fmt, ...)
{
  _ssize_t len;
  va_list arg;

  va_start(arg, fmt);
  len = vsnprintf(printf_buffer, PRINTF_BUF_MAX_LEN, fmt, arg);
  va_end(arg);

  if (len > PRINTF_BUF_MAX_LEN) {
    len = PRINTF_BUF_MAX_LEN - 1;
    printf_buffer[len] = 0;
  }

  if (len > 0)
    return (int)CDC_Transmit_FS((uint8_t*)printf_buffer, len);

  if (len == 0)
    return USBD_OK;

  return USBD_FAIL;
}

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

  if (text_sz <= 2)
    return;

  COMMAND_CHECK_CALL_ARG(ping)
  COMMAND_CHECK_CALL_ARG(meminfo)
  COMMAND_CHECK_CALL_ARG(timestamp)
  COMMAND_CHECK_CALL_ARG(setdate)
  COMMAND_CHECK_CALL_ARG(getdate)
  COMMAND_CHECK_CALL_ARG(help)
  COMMAND_CHECK_CALL_ARG(milliseconds)
  COMMAND_CHECK_CALL_ARG(cpuinfo)
  COMMAND_CHECK_CALL_ARG(sensors)

  usb_printf("Invalid command %.*s\n\n", text_sz, text);

}

volatile static int usbHasError = 0;

void usb_error(int value)
{
  usbHasError = value;
/*
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
*/
}

void usb_print_memory_info(void)
{
  get_flash_detailed(&df);
  get_ram_detailed(&dr);
#ifndef SOLAR48_DEBUG
  usb_printf(
    "RAM:\n\tTOTAL SIZE = %lu, TOTAL AVAILABLE = %lu, STACK USED = %lu, STACK PEAK USED = %lu, PERCENT USED = %lu%%\r\nFLASH:\n\tTOTAL SIZE = %lu, USED: %lu, PERCENT USED: %lu%%\r\n",
      dr.size, dr.total_available, dr.stack_used, dr.stack_peak_used, dr.percent_used,
      df.size, df.used, df.percent_used);
#else
  usb_printf(
    "RAM:\n\tTOTAL SIZE = %lu, TOTAL AVAILABLE = %lu, STACK USED = %lu, STACK PEAK USED = %lu, PERCENT USED = %lu%%\r\nFLASH:\n\tTOTAL SIZE = %lu, USED: %lu, PERCENT USED: %lu%% HEAP GAP RAM (DEBUG): %lu\r\n",
      dr.size, dr.total_available, dr.stack_used, dr.stack_peak_used, dr.percent_used,
      df.size, df.used, df.percent_used, dr.free_heap_stack_gap);
#endif
}

