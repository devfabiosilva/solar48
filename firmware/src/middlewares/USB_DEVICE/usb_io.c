#include <stdio.h>
#include <usb_device.h>
#include <memory.h>
#include <stdarg.h>
#include <usbd_cdc_if.h>
#include <console.h>
#include <string.h>
#include <process.h>
#include <errors.h>
#include <time.h>
#include <watchdog.h>

/*
sudo usermod -aG dialout $USER
minicom -D /dev/ttyACM0 -b 115200
screen /dev/ttyACM0 115200
cat /dev/ttyACM0
watch -n 0.5 "ls /dev/ttyACM*"
*/

#define COMMAND_CHECK_CALL_ARG(command) \
  if (strncmp(text, #command, sizeof(#command)-1) == 0) {\
    command##_cmd(text); \
    goto usb_receive_complete_process_finish; \
  }

DETAILED_RAM dr;
DETAILED_FLASH df;

extern error_callback_t usb_err_fn;
volatile bool usb_io_locked = false;

#define USB_IO_ERROR(error) ERROR_CALLBACK(usb_err_fn, error)

static char printf_buffer[APP_TX_DATA_SIZE];
#define PRINTF_BUF_MAX_LEN sizeof(printf_buffer)

static char text[APP_TX_DATA_SIZE];
size_t text_sz = 0;

#include <peripheral/ssd1306/oled_utils.h>
int n = 0;
volatile static int last_usb_error;
void usb_error(int value)
{
  last_usb_error = value;
// TODO Add error handler here
//  ssd1306_SetCursor(0, 40);
//  oled_printf("USBerr %d-%d", value, ++n);
}

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

  if (usb_io_locked) {
    iwd_refresh();
    USB_IO_ERROR(E_USB_LOCK_ERROR)
    return;
  }

  if (cdc_transmit_is_busy()) {
    iwd_refresh();
    USB_IO_ERROR(E_USB_RECEIVE_ERROR)
    return;
  }

  usb_io_locked = true;

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

int usb_receive_complete_process(void *ctx)
{

  int err = 0;
  uint64_t timeout_ms = 16;

  init_timeout_ms(&timeout_ms);

  if (text_sz <= 2) {
    err = -2;
    goto usb_receive_complete_process_finish;
  }

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

usb_receive_complete_process_finish:
  last_usb_error = 0;
  while (!is_timeout_ms(&timeout_ms));

  usb_io_locked = false;

  return err;
}

void usb_receive_complete()
{

  if (add_process_int_ext(usb_receive_complete_process, NULL))
    return;

  if (is_process_int_ext_running(usb_receive_complete_process)) {
    usb_error(E_USB_RECEIVE_PROC_BUSY);
  } else {
    usb_error(PROCESS_BUSY);
    usb_io_locked = false;
  }
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

