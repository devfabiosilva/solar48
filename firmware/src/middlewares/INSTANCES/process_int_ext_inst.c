#include <stdint.h>
#include <usb_io.h>
#include <process.h>
#include <instance_prio.h>
#include <solar48_config.h>

extern void halt();
static StaticTask_t processIntExtTaskTCB;
static StackType_t processIntExtTaskStack[ PROCESS_INT_EXT_STACK_SIZE ];

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
                                PROCESS_INT_EXT_STACK_SIZE,
                                NULL,
                                PRIO_4, // (Internal has 4 (max priority)) //PRIO_2
                                processIntExtTaskStack,
                                &( processIntExtTaskTCB ) )) return;

  usb_printf("Process int ext task error\n");
  halt();
}

