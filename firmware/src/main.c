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
//p panic_irq

extern void usb_receive(uint8_t *, uint32_t);
extern void usb_receive_complete();
extern void usb_error(int);
extern void halt();

void realtime(uint32_t);
SOLAR48_DATE sd;

void setup()
{
  __nvic_setprioritygrouping(NVIC_PRIORITYGROUP_4);

  init_usb_device(usb_receive, usb_receive_complete, usb_error);
  hal_i2c1_init();
  init_rtc(realtime);
#ifndef RTOS_SOLAR48 
  init_systick();
#endif
  init_gpios();
  init_sensors();

  init_idw();

#ifndef RTOS_SOLAR48
  END_SETUP
#endif
}

#ifdef RTOS_SOLAR48

void run(void)
{

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
  usb_printf("\nTimestamp: %u\n HAS RTOS TICKS: %d\n", time, has_rtos_ticks());
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

