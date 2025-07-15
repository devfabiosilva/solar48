#include <stdint.h>
#include <gpios.h>
#include <watchdog.h>
#include <usb_io.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

extern void halt();
static StaticTask_t exampleTaskTCB;
static StackType_t exampleTaskStack[ 1*configMINIMAL_STACK_SIZE ];

void led_blink_task(void *params)
{
  (void)params;

  while (1) {
    iwd_refresh();
    ledoff();
    vTaskDelay(pdMS_TO_TICKS(500));
//    usb_printf("\nPass2 ...\n\n");
    iwd_refresh();
    ledon();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void init_led_blink()
{
  if (xTaskCreateStatic( led_blink_task,
                                "led",
                                1*configMINIMAL_STACK_SIZE,
                                NULL,
                                1,
                                exampleTaskStack,
                                &( exampleTaskTCB ) )) return;

  usb_printf("Led task error!\n");
  halt();
}

