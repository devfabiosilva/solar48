#include <stdint.h>
#include <gpios.h>
#include <watchdog.h>
#include <usb_io.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
//#include <hal_i2c.h>

extern void halt();
static StaticTask_t exampleTaskTCB;
static StackType_t exampleTaskStack[ 1*configMINIMAL_STACK_SIZE ];

static void led_blink_task(void *params)
{
  (void)params;
/*
  static uint8_t data = 0xAE;
  uint8_t addr = 0;
  int err;
*/
  while (1) {
    iwd_refresh();
    ledoff();
    vTaskDelay(pdMS_TO_TICKS(500));
//    err = hal_i2c1_write(addr, 0, &data, 1, 10);
//    usb_printf("Addr %d and err = %d found %d\n", (int)addr++, err, (err > 10));
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

