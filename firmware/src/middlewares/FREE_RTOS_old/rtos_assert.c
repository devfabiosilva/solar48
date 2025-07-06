#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
//TODO remove it. Will be replaced by LCD panel driver
#include <usb_io.h>

void configASSERT_fn(int assert, char *msg)
{
  if ((assert) == 0) {
    usb_printf("\nKERNEL PANIC: %s", msg);
    taskDISABLE_INTERRUPTS();
    for( ;; );
  }
}

