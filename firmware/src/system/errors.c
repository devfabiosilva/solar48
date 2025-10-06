#include <sys_queue.h>
#include <solar48_config.h>
#include <stddef.h>

#ifdef USE_USB_PRINTF_IN_ERROR_LOGS
 #include <usb_io.h>
#else
 #include <peripheral/ssd1306/oled_utils.h>
#endif

SYS_QUEUE error_handler_queue = {0};
static int error_list[MAX_ERROR_LOGS_INSTANCES];

#define ERROR_LIST_BEGIN &error_list[0]
#define ERROR_LIST_END &error_list[MAX_ERROR_LOGS_INSTANCES - 1]

static int *error_list_ptr = ERROR_LIST_BEGIN;

int _error_handler(void *ctx)
{

#ifdef USE_USB_PRINTF_IN_ERROR_LOGS
  usb_printf("Error: %d\n", (int)*((int *)ctx));
#else
  oled_printf_cursor(0, 40, "Error: %d\n", (int)*((int *)*ctx));
#endif

  return 0;
}

void error_handler(int error)
{
  *error_list_ptr = error;
  if (sys_queue(&error_handler_queue, _error_handler, (void *)error_list_ptr, NULL))
    return;

  if (++error_list_ptr > ERROR_LIST_END)
    error_list_ptr = ERROR_LIST_BEGIN;
}

void init_error_handler_queue()
{
  sys_queue_init(&error_handler_queue, MAX_ERROR_LOGS_INSTANCES, false);
}

void run_error_handler()
{
  run_queue_run(&error_handler_queue);
}

