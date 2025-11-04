#include <stdint.h>
#include <usb_io.h>
#include <instance_prio.h>
#include <solar48_config.h>

extern void halt();
extern void rtos_milli_task(void *);
static StaticTask_t rtosMilliTaskTCB;
static StackType_t rtosMilliTaskStack[ PROCESS_RTOS_MILLI_STACK_SIZE ];

void init_rtos_milli()
{
  if (xTaskCreateStatic( rtos_milli_task,
                                "rtos_milli",
                                PROCESS_RTOS_MILLI_STACK_SIZE,
                                NULL,
                                PRIO_6, // PRIO_4
                                rtosMilliTaskStack,
                                &( rtosMilliTaskTCB ) )) return;

  usb_printf("rtos_milli task error!\n");
  halt();
}

