#include <stdint.h>
#include <usb_io.h>
#include <process.h>
#include <instance_prio.h>

extern void halt();
static StaticTask_t processIntExtTaskTCB;
static StackType_t processIntExtTaskStack[ 2*configMINIMAL_STACK_SIZE ];

static void run_proc_int_ext_task(void *params)
{
  (void)params;

  while (1) {
    //iwd_refresh();
    run_process_int_ext();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void init_process_int_ext_task()
{
  if (xTaskCreateStatic( run_proc_int_ext_task,
                                "processIntExt",
                                2*configMINIMAL_STACK_SIZE,
                                NULL,
                                PRIO_2, // (Internal has 2 (max priority))
                                processIntExtTaskStack,
                                &( processIntExtTaskTCB ) )) return;

  usb_printf("Process int ext task error\n");
  halt();
}

