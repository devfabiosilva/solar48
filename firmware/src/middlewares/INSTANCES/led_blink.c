#include <stdint.h>
#include <gpios.h>
#include <watchdog.h>
#include <usb_io.h>
#include <instance_prio.h>

//#define UART_TEST
#define RS485_TEST

#ifdef RS485_TEST
 #include <rs485.h>
 #include <peripheral/ssd1306/oled_utils.h>

static uint16_t data[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
};
#endif

#ifdef UART_TEST
 #include <hal_uart.h>
 #include <peripheral/ssd1306/oled_utils.h>
#endif

extern void halt();
static StaticTask_t exampleTaskTCB;
static StackType_t exampleTaskStack[ 2*configMINIMAL_STACK_SIZE ];
//uxTaskGetStackHighWaterMark
//configCHECK_FOR_STACK_OVERFLOW
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

#ifdef RS485_TEST
void rs485_receive(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  switch (status) {
    case MASTER_TRANSFER_SUCCESS:
      oled_cursor_printf(0, 50, "Success");
      break;
    default:
      oled_cursor_printf(0, 50, "Fail %d", status);
  }
}
#endif

static void led_blink_task(void *params)
{
  (void)params;

#ifdef UART_TEST
  static uint64_t cnt = 0;
  enum uart_status_t uart_status;
#endif

#ifdef RS485_TEST
  int status;
  uint8_t slv_addr = 0xAA;
  MB_FUNCTION fc = WRITE_MULTIPLE_REGISTERS;
  uint16_t mem_address = 0x0102;
  uint16_t n = (uint16_t)sizeof(data);
#endif

  BaseType_t uxHighWaterMark, uxHighWaterMark_max = 0;

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
      oled_cursor_printf(0, 40, "UART send %lu", (int)cnt);
    else
      oled_cursor_printf(0, 40, "UART error %d", (int)uart_status);

    ++cnt;
#endif

  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL)*sizeof(StackType_t); 
  if (uxHighWaterMark > uxHighWaterMark_max)
    uxHighWaterMark_max = uxHighWaterMark;
#ifdef RS485_TEST

    status = master_send_req(slv_addr, fc, mem_address, n, 0, 0, (void *)&data[0], 40, rs485_receive);
    if (status == RS485_OK)
      oled_cursor_printf(0, 40, "RS485 %d|%d", (int)uxHighWaterMark, (int)uxHighWaterMark_max);
    else
      oled_cursor_printf(0, 40, "RS485 error %d", (int)status);
#endif
  }
}

void init_led_blink()
{
  if (xTaskCreateStatic( led_blink_task,
                                "led",
                                2*configMINIMAL_STACK_SIZE,
                                NULL,
                                PRIO_0,
                                exampleTaskStack,
                                &( exampleTaskTCB ) )) return;

  usb_printf("Led task error!\n");
  halt();
}

