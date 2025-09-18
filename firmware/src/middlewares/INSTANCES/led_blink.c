#include <stdint.h>
#include <gpios.h>
#include <watchdog.h>
#include <usb_io.h>
#include <instance_prio.h>

#define UART_TEST
#ifdef UART_TEST
 #include <hal_uart.h>
 #include <peripheral/ssd1306/oled_utils.h>
#endif

extern void halt();
static StaticTask_t exampleTaskTCB;
static StackType_t exampleTaskStack[ 1*configMINIMAL_STACK_SIZE ];

#ifdef UART_TEST

void uart_rcv(int status)
{
  switch (status) {
    case UART1_TRANSFER_COMPLETE:
      oled_cursor_printf(0, 50, "Success");
      break;
    default:
      oled_cursor_printf(0, 50, "Fail");
  }
}

#endif

static void led_blink_task(void *params)
{
  (void)params;

#ifdef UART_TEST
  static uint8_t cnt = 0;
  enum uart_status_t uart_status;
#endif
  while (1) {
    iwd_refresh();
    ledoff();
    vTaskDelay(pdMS_TO_TICKS(500));
    iwd_refresh();
    ledon();
    vTaskDelay(pdMS_TO_TICKS(500));
#ifdef UART_TEST
    uart_status = uart1_transmit((uint8_t *)&cnt, sizeof(cnt), uart_rcv, 10);
    if (uart_status == UART_OK)
      oled_cursor_printf(0, 40, "UART send %d", (int)cnt);
    else
      oled_cursor_printf(0, 40, "UART error %d", (int)uart_status);

    ++cnt;
#endif
  }
}

void init_led_blink()
{
  if (xTaskCreateStatic( led_blink_task,
                                "led",
                                1*configMINIMAL_STACK_SIZE,
                                NULL,
                                PRIO_0,
                                exampleTaskStack,
                                &( exampleTaskTCB ) )) return;

  usb_printf("Led task error!\n");
  halt();
}

