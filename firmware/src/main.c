#include <system.h>
#include <usb_device.h>
#include <rtc.h>
#include <usb_io.h>
#include <gpios.h>
#include <time.h>
#include <watchdog.h>
#include <sensors.h>
#include <hal_i2c.h>
#include <hal_uart.h>
#include <registers.h>
#include <process.h>
#include <peripheral/ssd1306/oled_utils.h>
#include <errors.h>

#ifdef IMPLEMENT_RS485_MASTER_OVER_UART1
#include <rs485.h>
#endif

#ifdef RTOS_SOLAR48

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <instances.h>

#endif

void realtime(uint32_t);
SOLAR48_DATE sd;

void setup()
{
  __nvic_setprioritygrouping(NVIC_PRIORITYGROUP_4);

  init_usb();
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
  init_uart1(UART1_DEFAULT_SPEED, PARITY_DISABLE);
#else
  init_master_rs485(speed_115_2_kbps, PARITY_DISABLE);
#endif

#ifndef IMPLEMENT_RS485_SLAVE_OVER_UART2
  init_uart2(UART2_DEFAULT_SPEED, PARITY_DISABLE2);
#else
  init_slave_rs485(10, speed_115_2_kbps, PARITY_DISABLE2, 100, false);
#endif
  hal_i2c1_init();
  init_rtc(realtime);

  init_systick(); // If RTOS_SOLAR48 so this is used temporary for update peripherals timing systems

  init_gpios();
  init_sensors();

  init_idw();

  END_SETUP // Enable all interrupts
}

#ifdef RTOS_SOLAR48

void run(void)
{

  init_oled("Solar48rtos");

  DISABLE_SETUP // Disable all interrupts

  init_rtos_milli();
  init_led_blink();
  init_process_task();
  init_process_int_ext_task();
  init_process_int_int_task();
  init_process_periph_evt_task();
  init_sys_queue_task();

  vTaskStartScheduler();
}

#else

uint64_t uart_timeout;
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
uint64_t cnt = 0;

void uart_rcv(int status)
{
   switch (status) {
     case UART1_TRANSFER_COMPLETE:
       oled_cursor_printf(0, 50, "Success");
       break;
     default:
       oled_cursor_printf(0, 50, "Fail %d", status);
   }
}
void test_uart1_transmit()
#else
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

uint8_t slv_addr = 0xAA;
MB_FUNCTION fc = WRITE_MULTIPLE_REGISTERS;
uint16_t mem_address = 0x0102;
static uint16_t data[] = {
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32
};

uint16_t n = (uint16_t)sizeof(data);

void test_rs485_trasmit()
#endif
{
  if (milliseconds() > uart_timeout) {
    uart_timeout = milliseconds() + 500;
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
    enum uart_status_t uart_status;

    uart_status = uart1_transmit((uint8_t *)&cnt, sizeof(cnt), uart_rcv, 10);
    if (uart_status == UART_OK)
      oled_cursor_printf(0, 40, "UART send byte %d", (int)cnt);
    else
      oled_cursor_printf(0, 40, "UART error %d", (int)uart_status);

    ++cnt;

#else

    int status = master_send_req(slv_addr, fc, mem_address, n, 0, 0, (void *)&data[0], 40, rs485_receive);
    if (status == RS485_OK)
      oled_cursor_printf(0, 40, "RS485 success");
    else
      oled_cursor_printf(0, 40, "RS485 error %d", status);

#endif
  }
}

extern void _run_oled_process();
void run(void)
{
//  usb_printf("\nInitializing ...\n\nPriority group: %u\n", get_priority_grouping());

  init_oled("Solar48bm");
  _run_oled_process();

  blink_n(1);
  usb_printf("\nReady ...\n\n");
  uart_timeout = milliseconds() + 200;
  init_error_handler_queue();
  while (1) {
    run_process_int_int();
    process_uart1_time_event();
    process_uart2_time_event();
    run_process_int_ext();
    run_process();
#ifndef IMPLEMENT_RS485_MASTER_OVER_UART1
    test_uart1_transmit();
#else
    test_rs485_trasmit();
#endif
    _run_oled_process();
    run_error_handler();
    delay(1);
  }
}

#endif

#ifdef RTOS_SOLAR48
//TODO refactor. Will be removed to rtc.c

//#define SHOW_ALL

int print_text(void *ctx)
{
  get_solar48_date(&sd, NULL);

  ssd1306_SetCursor(0, 18);

  int oled_error =
#ifdef SHOW_ALL
  oled_cursor_printf(0, 18,
    "%s-%u/%u/%u\n%02d:%02d:%02d\nTemp: %0.2f oC\nVolt.: %0.2f V",
    get_day_str((int)sd.year, (int)sd.month, (int)sd.day), sd.year, (int)sd.month, (int)sd.day,
    (int)sd.hour, (int)sd.minute, (int)sd.second, read_internal_temp_sensor(), read_vref()
  );
#else
  oled_cursor_printf(0, 18,
    "%s-%u/%u/%u\n%02d:%02d:%02d",
    get_day_str((int)sd.year, (int)sd.month, (int)sd.day), sd.year, (int)sd.month, (int)sd.day,
    (int)sd.hour, (int)sd.minute, (int)sd.second
  );
#endif

  if (oled_error)
    usb_printf("Oled Error %d\n", oled_error);

  return 0;
}

void realtime(uint32_t time)
{
  if (oled_is_initialized()) {
    if (!add_process_int_int(print_text, NULL))
      usb_printf("\nRealtime: Process busy\n");
  } else
    usb_printf("Oled not initialized");
}

#else

//TODO refactor. Will be removed to rtc.c
volatile int blink = 0;
void realtime(uint32_t time)
{

  get_solar48_date(&sd, NULL);

  oled_cursor_printf(0, 18,
    "%s-%u/%u/%u\n%02d:%02d:%02d",
    get_day_str((int)sd.year, (int)sd.month, (int)sd.day), sd.year, (int)sd.month, (int)sd.day,
    (int)sd.hour, (int)sd.minute, (int)sd.second
  );

  //oled_printf("\nTemp: %0.2f oC\nVolt.: %0.2f V", read_internal_temp_sensor(), read_vref());

  if (blink) {
    ledon();
    blink = 0;
  } else {
    ledoff();
    blink = 1;
  }
}
#endif

