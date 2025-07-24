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
#include <peripheral/ssd1306/ssd1306.h>
#include <registers.h>

#ifdef RTOS_SOLAR48

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <instances.h>

#else

#include <usb_io.h>
#include <process.h>

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

#ifndef RTOS_SOLAR48
  END_SETUP // Enable all interrupt for non RTOS system
#endif

// init peripherals below

// OLED Panel
//  oled_fail = ssd1306_Init();

}

#ifdef RTOS_SOLAR48
volatile static int err = 0;
void run(void)
{

  END_SETUP
  //err = hal_i2c1_write(12, 3, &data, 1, 100);
  //err = hal_i2c1_write(0xAA, 4, &data, 1, 200);
  err = ssd1306_Init();

  //DISABLE_SETUP
  //ssd1306_WriteString("Test", Font_11x18, White);
//  usb_printf("oled return %d", err);
  //  ssd1306_WriteString("Test", Font_11x18, White);
  init_led_blink();
  init_process_task();

  vTaskStartScheduler();
}

#else

void run(void)
{
  usb_printf("\nInitializing ...\n\nPriority group: %u\n", get_priority_grouping());

  blink_n(3);
  usb_printf("\nReady ...\n\n");
  while (1) {
    run_process();
    delay(1);
  }
}

#endif

#ifdef RTOS_SOLAR48
//TODO refactor. Will be removed to rtc.c
void realtime(uint32_t time)
{
  usb_printf("\nTimestamp: %u\n HAS RTOS TICKS: %d oled %d\n", time, has_rtos_ticks(), err);
}
#else

//TODO refactor. Will be removed to rtc.c
volatile int blink = 0;
void realtime(uint32_t time)
{
  uint32_t tm = time;
  get_solar48_date(&sd, &tm);

  usb_printf("\n\nTIME: %u:%u:%u\n\n", sd.hour, (uint32_t)sd.minute, (uint32_t)sd.second);
  usb_printf("\n\nDay (yyyy/mm/dd): %s - %u/%u/%u\n\n", get_day_str((int)sd.year, (int)sd.month, (int)sd.day), sd.year, (uint32_t)sd.month, (uint32_t)sd.day);

  if (blink) {
    ledon();
    blink = 0;
  } else {
    ledoff();
    blink = 1;
  }
}
#endif

