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
  init_uart1();
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

uint64_t cnt = 0;
uint64_t uart_timeout;
void test_uart1_transmit()
{
  if (milliseconds() > uart_timeout) {
    uart_timeout = milliseconds() + 500;

    enum uart_status_t uart_status;

    uart_status = uart1_transmit((uint8_t *)&cnt, sizeof(cnt), uart_rcv, 10);
    if (uart_status == UART_OK)
      oled_cursor_printf(0, 40, "UART send byte %d", (int)cnt);
    else
      oled_cursor_printf(0, 40, "UART error %d", (int)uart_status);

    ++cnt;
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
  while (1) {
    run_process_int_int();
    process_uart1_time_event();
    run_process_int_ext();
    run_process();
    test_uart1_transmit();
    _run_oled_process();
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

