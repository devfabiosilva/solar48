#include <stdint.h>
#include <gpios.h>
#include <watchdog.h>
#include <usb_io.h>
#include <instance_prio.h>
#include <solar48_config.h>

//#define UART_TEST
//#define RS485_TEST
//#define UART2_TEST

//#define TEST_RS485_SLAVE_AND_MASTER

#ifdef TEST_RS485_SLAVE_AND_MASTER
 #include <rs485.h>
 #include <peripheral/ssd1306/oled_utils.h>

extern RS485_HOLDING_REGISTERS_MEMORY_AREA rs485_slave_holding_register_memory_area;

#define SLAVE_ADDRESS 10
#define HLD_REG_ADDR 0x4000

#endif


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

#ifdef UART2_TEST
 #include <hal_uart.h>
 #include <peripheral/ssd1306/oled_utils.h>
#endif

extern void app_panic(const char *);
static StaticTask_t exampleTaskTCB;
static StackType_t exampleTaskStack[ LED_BLINK_STACK_SIZE ];
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

#ifdef UART2_TEST

void uart2_rcv(int status)
{
  switch (status) {
    case UART2_TRANSFER_COMPLETE:
      oled_cursor_printf(0, 50, "Success2");
      break;
    default:
      oled_cursor_printf(0, 50, "Fail2 %d", status);
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

#ifdef TEST_RS485_SLAVE_AND_MASTER

static void populate_and_init_uart_test()
{

  uint32_t x = 0x00020001;
  size_t index = 0;
  uint32_t *ptr = (uint32_t *)&rs485_slave_holding_register_memory_area.sector000;

  while (index < sizeof(rs485_slave_holding_register_memory_area)/sizeof(uint32_t)) {
    *(ptr++) = x;
    ++index;
    x += 0x00020002;
  }

  rs485_slave_start_listen();
}

volatile static int resp = 0;

void rs485_master_receive(int status, MB_FUNCTION function, uint8_t *data, uint16_t data_size)
{
  switch (status) {
    case MASTER_TRANSFER_SUCCESS:
    //case UART1_RECEIVE_AND_PROCESS_DATA_COMPLETE:
      resp = 0;
      if (data_size != 8)
        resp = -10;
      else if (data == NULL)
        resp = -11;
      else {
        uint16_t *u16 = (uint16_t *)data;
        if (*(u16++) != 0x0203)
          resp = -12;
          
        if ((resp == 0) && *(u16++) != 0x0001)
          resp = -13;
          
        if ((resp == 0) && *(u16++) != 0x0607)
          resp = -14;

        if ((resp == 0) && *(u16++) != 0x0405)
          resp = -15;

        if ((resp == 0) && *(u16++) != 0x0a0b)
          resp = -16;

        if ((resp == 0) && *(u16++) != 0x0809)
          resp = -17;

        if ((resp == 0) && *(u16++) != 0x0e0f)
          resp = -18;

        if ((resp == 0) && *(u16++) != 0x0c0d)
          resp = -19;
      }

      oled_cursor_printf(0, 50, "OK %d", status);
      break;
    default:
      if (status != 3) {
        resp = status;
        oled_cursor_printf(0, 50, "Fail %d", status);
      } else
        oled_cursor_printf(0, 50, "OK '0'");
  }
}

#endif

static void led_blink_task(void *params)
{
  (void)params;

#ifdef UART_TEST
  static uint64_t cnt = 0;
  enum uart1_status_t uart_status;
#endif

#ifdef UART2_TEST
  static uint64_t cnt2 = 0;
  enum uart2_status_t uart2_status;
#endif

#ifdef RS485_TEST
  int status;
  uint8_t slv_addr = 0xAA;
  MB_FUNCTION fc = WRITE_MULTIPLE_REGISTERS;
  uint16_t mem_address = 0x0102;
  uint16_t n = (uint16_t)sizeof(data);
#endif

#ifdef TEST_RS485_SLAVE_AND_MASTER
  int status;
  populate_and_init_uart_test();
#endif

  //BaseType_t uxHighWaterMark, uxHighWaterMark_max = 0;

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

#ifdef UART2_TEST
    uart2_status = uart2_transmit((uint8_t *)&cnt2, sizeof(cnt2), uart2_rcv, 10);
    if (uart2_status == UART2_OK)
      oled_cursor_printf(0, 40, "UART2 send %lu", (int)cnt2);
    else
      oled_cursor_printf(0, 40, "UART2 error %d", (int)uart2_status);

    ++cnt2;
#endif

/*
  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL)*sizeof(StackType_t); 
  if (uxHighWaterMark > uxHighWaterMark_max)
    uxHighWaterMark_max = uxHighWaterMark;
*/
#ifdef RS485_TEST

    status = master_send_req(slv_addr, fc, mem_address, n, 0, 0, (void *)&data[0], 40, rs485_receive);
    if (status == RS485_OK) {
      //oled_cursor_printf(0, 40, "RS485 %d|%d", (int)uxHighWaterMark, (int)uxHighWaterMark_max);
      oled_cursor_printf(0, 40, "RS485 resp=%d", resp);
    } else
      oled_cursor_printf(0, 40, "RS485 error %d", (int)status);
#endif

#ifdef TEST_RS485_SLAVE_AND_MASTER
    status = MASTER_READ_HOLDING_REGISTERS(SLAVE_ADDRESS, HLD_REG_ADDR, 8, 10, rs485_master_receive);
    if (status == RS485_OK) {
      //oled_cursor_printf(0, 40, "RS485 %d|%d", (int)uxHighWaterMark, (int)uxHighWaterMark_max);
      oled_cursor_printf(0, 40, "RS485 resp=%d", resp);
    } else
      oled_cursor_printf(0, 40, "RS485 error %d", (int)status);
#endif

  }
}

void init_led_blink()
{
  if (xTaskCreateStatic( led_blink_task,
                                "led",
                                LED_BLINK_STACK_SIZE,
                                NULL,
                                PRIO_0,
                                exampleTaskStack,
                                &( exampleTaskTCB ) )) return;

  app_panic("ledinit");
}

