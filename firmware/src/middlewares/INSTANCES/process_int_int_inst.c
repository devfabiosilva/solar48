#include <stdint.h>
#include <usb_io.h>
#include <process.h>
#include <instance_prio.h>

extern void halt();
static StaticTask_t processIntExtTaskTCB;
static StackType_t processIntExtTaskStack[ 2*configMINIMAL_STACK_SIZE ];

static void run_proc_int_int_task(void *params)
{
  (void)params;

  while (1) {
    //iwd_refresh();
    run_process_int_int();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void init_process_int_int_task()
{
  if (xTaskCreateStatic( run_proc_int_int_task,
                                "processIntInt",
                                2*configMINIMAL_STACK_SIZE,
                                NULL,
                                PRIO_3, //(Internal has 3 (max priority))
                                processIntExtTaskStack,
                                &( processIntExtTaskTCB ) )) return;

  usb_printf("Process int int task error\n");
  halt();
}

