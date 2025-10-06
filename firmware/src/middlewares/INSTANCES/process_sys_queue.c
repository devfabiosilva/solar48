#include <stdint.h>
#include <usb_io.h>
#include <instance_prio.h>
#include <sys_queue.h>
#include <errors.h>

extern void halt();
extern void _run_oled_process();
static StaticTask_t processTaskSysQueueTCB;
static StackType_t processTaskSysQueueStack[ 2*configMINIMAL_STACK_SIZE ];

extern void run_error_handler();

static void run_proc_sys_queue_task(void *params)
{
  (void)params;

  while (1) {
    _run_oled_process();
    run_error_handler();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void init_sys_queue_task()
{

  init_error_handler_queue();
  if (xTaskCreateStatic( run_proc_sys_queue_task,
                                "sysqueue_proc",
                                2*configMINIMAL_STACK_SIZE,
                                NULL,
                                PRIO_2, // PRIO_1
                                processTaskSysQueueStack,
                                &( processTaskSysQueueTCB ) )) return;

  usb_printf("Process sys queue error\n");
  halt();
}

