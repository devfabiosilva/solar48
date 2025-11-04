#include <stdint.h>
#include <usb_io.h>
#include <process.h>
#include <instance_prio.h>
#include <solar48_config.h>

extern void halt();
static StaticTask_t processTaskTCB;
static StackType_t processTaskStack[ PROCESS_INST_STACK_SIZE ];

static void run_proc_task(void *params)
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
                                PROCESS_INST_STACK_SIZE,
                                NULL,
                                PRIO_3, // PRIO_1
                                processTaskStack,
                                &( processTaskTCB ) )) return;

  usb_printf("Process task error\n");
  halt();
}

