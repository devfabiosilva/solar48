#include <stdio.h>
#include <usb_device.h>
#include <memory.h>
#include <stdarg.h>
#include <usbd_cdc_if.h>

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

// Return USBD_OK if transmit was success, else error
uint8_t usb_printf(const char *fmt, ...)
{
  size_t len;
  va_list arg;

  va_start(arg, fmt);
  len = vsnprintf(printf_buffer, PRINTF_BUF_MAX_LEN, fmt, arg);
  va_end(arg);

  if (len > PRINTF_BUF_MAX_LEN)
    len = PRINTF_BUF_MAX_LEN;

  if (len)
    return CDC_Transmit_FS((uint8_t*)printf_buffer, len);

  return USBD_OK;
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

