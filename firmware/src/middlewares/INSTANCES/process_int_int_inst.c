#include <stdint.h>
#include <usb_io.h>
#include <process.h>
#include <instance_prio.h>
#include <solar48_config.h>

extern void app_panic(const char *);
static StaticTask_t processIntIntTaskTCB;
static StackType_t processIntIntTaskStack[ PROCESS_INT_INT_STACK_SIZE ];

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
                                PROCESS_INT_INT_STACK_SIZE,
                                NULL,
                                PRIO_5, //(Internal has 5 (max priority)) // PRIO_3
                                processIntIntTaskStack,
                                &( processIntIntTaskTCB ) )) return;

  app_panic("procIntIntInit");
}

