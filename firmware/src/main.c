#include <system.h>
#include <usb_device.h>
#include <rtc.h>
#include <usb_io.h>
#include <gpios.h>
#include <time.h>
#include <watchdog.h>
#include <sensors.h>
#include <registers.h>
#include <hal_i2c.h>
#include <registers.h>
#include <process.h>
#include <peripheral/ssd1306/oled_utils.h>

#ifdef RTOS_SOLAR48

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <instances.h>

#else

#include <usb_io.h>

#endif

//dmesg -w
//sudo modprobe usbmon
//sudo cat /sys/kernel/debug/usb/usbmon/1u
// lsusb -d 0483:5740 -v | grep -iE 'manufacturer|product|serial'
// GDB target remote :3333
//info registers
//monitor reset halt
//p panic_irq

extern void usb_receive(uint8_t *, uint32_t);
extern void usb_receive_complete();
extern void usb_error(int);
extern void halt();

//static int oled_fail = 0;

void realtime(uint32_t);
SOLAR48_DATE sd;
//https://community.st.com/t5/stm32-mcus-products/stm32f107-i2c-scl-stays-low/m-p/301186#M70029
void setup()
{
  __nvic_setprioritygrouping(NVIC_PRIORITYGROUP_4);

  init_usb_device(usb_receive, usb_receive_complete, usb_error);
  hal_i2c1_init();
  init_rtc(realtime);
//#ifndef RTOS_SOLAR48 
  init_systick(); // If RTOS_SOLAR48 so this is used temporary for update peripherals timing systems
//#endif
  init_gpios();
  init_sensors();

  init_idw();

  END_SETUP // Enable all interrupts
}

volatile static int err = 0;
void init_oled(char *msg)
{
  if ((err = ssd1306_Init()))
    return;

  ssd1306_WriteString(msg, Font_11x18, White);
  ssd1306_UpdateScreen();

}

#ifdef RTOS_SOLAR48

void run(void)
{

  init_oled("Solar48rtos");

  DISABLE_SETUP // Disable all interrupts

  init_rtos_milli();
  init_led_blink();
  init_process_task();

  vTaskStartScheduler();
}

#else

void run(void)
{
//  usb_printf("\nInitializing ...\n\nPriority group: %u\n", get_priority_grouping());

  init_oled("Solar48bm");

  blink_n(3);
  usb_printf("\nReady ...\n\n");
  while (1) {
    run_process();
    delay(1);
  }
}

#endif

static char buf[20];
#include <stdio.h>

//TODO remove. For testing only
#define USE_OLED_PRINTF

int print_text(void *ctx)
{
  get_solar48_date(&sd, NULL);

#ifdef USE_OLED_PRINTF
  ssd1306_SetCursor(0, 18);
  oled_printf(
    "%s-%u/%u/%u\n%02d:%02d:%02d",
    get_day_str((int)sd.year, (int)sd.month, (int)sd.day), sd.year, (int)sd.month, (int)sd.day,
    (int)sd.hour, (int)sd.minute, (int)sd.second
  );
//  ssd1306_SetCursor(0, 28);
//  err = oled_printf("%02d:%02d:%02d", (int)sd.hour, (int)sd.minute, (int)sd.second);
#else
  snprintf(buf, sizeof(buf), "%s-%u/%u/%u", get_day_str((int)sd.year, (int)sd.month, (int)sd.day), sd.year, (int)sd.month, (int)sd.day);
  ssd1306_SetCursor(0, 18);
  ssd1306_WriteString(buf, Font_7x10, White);
  ssd1306_SetCursor(0, 28);
  snprintf(buf, sizeof(buf), "%02d:%02d:%02d", (int)sd.hour, (int)sd.minute, (int)sd.second);
  ssd1306_WriteString(buf, Font_7x10, White);
  ssd1306_UpdateScreen();
#endif

  return 0;
}

#ifdef RTOS_SOLAR48
//TODO refactor. Will be removed to rtc.c

void realtime(uint32_t time)
{
  if (!err) {
    if (!add_process(print_text, NULL))
      usb_printf("\nProcess busy\n");
  } else {
    usb_printf("Error %d\n", err);
    err = 0;
  }
  usb_printf("Time %u\n", time);
}
#else

//TODO refactor. Will be removed to rtc.c
volatile int blink = 0;
void realtime(uint32_t time)
{
/*
  uint32_t tm = time;
  get_solar48_date(&sd, &tm);

  usb_printf("\n\nTIME: %u:%u:%u\n\n", sd.hour, (uint32_t)sd.minute, (uint32_t)sd.second);
  usb_printf("\n\nDay (yyyy/mm/dd): %s - %u/%u/%u\n\n", get_day_str((int)sd.year, (int)sd.month, (int)sd.day), sd.year, (int)sd.month, (int)sd.day);
*/
  get_solar48_date(&sd, NULL);

  ssd1306_SetCursor(0, 18);
  oled_printf(
    "%s-%u/%u/%u\n%02d:%02d:%02d",
    get_day_str((int)sd.year, (int)sd.month, (int)sd.day), sd.year, (int)sd.month, (int)sd.day,
    (int)sd.hour, (int)sd.minute, (int)sd.second
  );

  if (blink) {
    ledon();
    blink = 0;
  } else {
    ledoff();
    blink = 1;
  }
}
#endif

