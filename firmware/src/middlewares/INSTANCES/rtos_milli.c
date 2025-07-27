#include <stdint.h>
#include <usb_io.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

extern void halt();
extern void rtos_milli_task(void *);
static StaticTask_t rtosMilliTaskTCB;
static StackType_t rtosMilliTaskStack[ 1*configMINIMAL_STACK_SIZE ];

void init_rtos_milli()
{
  if (xTaskCreateStatic( rtos_milli_task,
                                "rtos_milli",
                                1*configMINIMAL_STACK_SIZE,
                                NULL,
                                configMAX_PRIORITIES - 1,
                                rtosMilliTaskStack,
                                &( rtosMilliTaskTCB ) )) return;

  usb_printf("rtos_milli task error!\n");
  halt();
}

