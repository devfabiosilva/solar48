#include <stdint.h>
#include <usb_io.h>
#include <process.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

extern void halt();
static StaticTask_t processTaskTCB;
static StackType_t processTaskStack[ 2*configMINIMAL_STACK_SIZE ];

void run_proc_task(void *params)
{
  (void)params;

  while (1) {
    //iwd_refresh();
    run_process();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void init_process_task()
{
  if (xTaskCreateStatic( run_proc_task,
                                "process",
                                2*configMINIMAL_STACK_SIZE,
                                NULL,
                                2,
                                processTaskStack,
                                &( processTaskTCB ) )) return;

  usb_printf("Process task error\n");
  halt();
}

