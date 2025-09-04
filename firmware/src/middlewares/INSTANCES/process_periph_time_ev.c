#include <stdint.h>
#include <usb_io.h>
#include <instance_prio.h>


extern void halt();
extern void process_uart1_time_event();
static StaticTask_t processPeriphEvtTaskTCB;
static StackType_t processPeriphEvtTaskStack[ 2*configMINIMAL_STACK_SIZE ];

static void run_process_periph_evt_task(void *params)
{
  (void)params;

  while (1) {
    process_uart1_time_event();
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

void init_process_periph_evt_task()
{
  if (xTaskCreateStatic( run_process_periph_evt_task,
                                "periphExtEvent",
                                2*configMINIMAL_STACK_SIZE,
                                NULL,
                                PRIO_2, // (Internal has 2 (max priority))
                                processPeriphEvtTaskStack,
                                &( processPeriphEvtTaskTCB ) )) return;

  usb_printf("External event time task error\n");
  halt();
}

