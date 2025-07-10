#include <stddef.h>
#include <system.h>
#include <usb_device.h>
#include <rtc.h>
#include <usb_io.h>
#include <gpios.h>
#include <time.h>
#include <errors.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <console.h>
#include <watchdog.h>
#include <sensors.h>
#include <process.h>
//#include <interrupt_panic.h>
#include <registers.h>

#ifdef RTOS_SOLAR48

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#endif

//#include <stdlib.h>
//dmesg -w
//sudo modprobe usbmon
//sudo cat /sys/kernel/debug/usb/usbmon/1u
// lsusb -d 0483:5740 -v | grep -iE 'manufacturer|product|serial'
// GDB target remote :3333
//https://arm-software.github.io/CMSIS-FreeRTOS/main/page_create_project.html#configure_native_freertos
//https://github.com/ARM-software/CMSIS-FreeRTOS/blob/main/Source/portable/GCC/ARM_CM3/portmacro.h
//https://community.st.com/t5/stm32-mcus/how-to-debug-a-hardfault-on-an-arm-cortex-m-stm32/ta-p/672235
//https://forums.freertos.org/t/svc-call-causing-hardfault-when-there-is-no-pre-emption-priority/20827/3
//https://forums.freertos.org/t/svc-call-causing-hardfault-when-there-is-no-pre-emption-priority/20827/7
//https://community.st.com/t5/stm32-mcus-embedded-software/freertos-default-cubemx-project-hardfault/td-p/272792
//https://community.st.com/t5/stm32cubemx-mcus/stm32f407g-disc1-the-project-template-doesn-t-work-with-rtos/m-p/317646
//https://forums.freertos.org/t/prvportstartfirsttask-fails-at-svc-0/18288/18
//https://forums.freertos.org/t/application-stops-when-executing-prvportstartfirsttask-for-cm4/18338/6
//https://freertos.org/Documentation/02-Kernel/03-Supported-devices/04-Demos/ARM-Cortex/RTOS-Cortex-M3-M4
//https://developer.arm.com/documentation/dui0646/c/The-Cortex-M7-Processor/Exception-model/Exception-entry-and-return
//info registers
//p panic_irq
volatile int hasError = 0;
void usb_receive(uint8_t *, uint32_t);
void usb_receive_complete();
void usb_error(int);
void realtime(uint32_t);
SOLAR48_DATE sd;

void setup()
{
  __nvic_setprioritygrouping(NVIC_PRIORITYGROUP_4);
  //__asm volatile("svc 0\n");
#ifdef RTOS_SOLAR48
//     __nvic_set_priority(SysTick_IRQn, 22);
//     __nvic_set_priority(PendSV_IRQn, 20);
#endif
  init_usb_device(usb_receive, usb_receive_complete, usb_error);
  init_rtc(realtime);
#ifndef RTOS_SOLAR48 
  init_systick();
#endif
  init_gpios();
  init_sensors();

  init_idw();

//END_SETUP // TODO remove
#ifndef RTOS_SOLAR48 
  END_SETUP
#endif
}

#ifdef RTOS_SOLAR48
#include <core_cm3.h>

void led_blink_task(void *params)
{
  (void)params;
//  usb_printf("\nRODOU\n");
  while (1) {
    //iwd_refresh();
    //usb_printf("\nPass1 ...\n\n");
    //blink_n(3);
    ledoff();
    vTaskDelay(pdMS_TO_TICKS(1000));
    //usb_printf("\nPass2 ...\n\n");
    ledon();
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void run(void)
{
/*
  TaskHandle_t led_task_handle = NULL;
  usb_printf("\nInitializing ...\n\nPriority group: %u\n", get_priority_grouping());
  //usb_printf("\nReady ...\n\n");
//  blink_n(1);
  usb_printf("SYSTICK CTRL 1: 0x%08lx\n", SysTick->CTRL);

  BaseType_t err;
  if ((err=xTaskCreate(led_blink_task, "LED", 2*128, NULL, 1, &led_task_handle)) != pdPASS) {
    usb_printf("\nUnable to create task %d ...\n\n", err);
    goto fail;
  }
*/
  static StaticTask_t exampleTaskTCB;
  static StackType_t exampleTaskStack[ 4*configMINIMAL_STACK_SIZE ];

  TaskHandle_t task_handle = xTaskCreateStatic( led_blink_task,
                                "led",
                                2*configMINIMAL_STACK_SIZE,
                                NULL,
                                configMAX_PRIORITIES - 1U,
                                //&( exampleTaskStack[ 0 ] ),
                                exampleTaskStack,
                                &( exampleTaskTCB ) );


if (task_handle == NULL) {
    usb_printf("Erro ao criar a tarefa!\n");
    while(1);
}

//  usb_printf("Free heap: %u\n", xPortGetFreeHeapSize());
  //usb_printf("Min ever free heap: %u\n", xPortGetMinimumEverFreeHeapSize());
//  usb_printf("SYSTICK CTRL 2: 0x%08lx\ntask_handle %p\n", SysTick->CTRL, led_task_handle);
  //usb_printf("Free heap: %u\n", xPortGetFreeHeapSize());
  //usb_printf("Min ever free heap: %u\n", xPortGetMinimumEverFreeHeapSize());
  //blink_n(2);
//  usb_printf("SHPR2 (SVCall): 0x%08lx\nSHPR3 (PendSV/SysTick): 0x%08lx\n\n", *(volatile uint32_t *)0xE000ED1C, *(volatile uint32_t *)0xE000ED20);
  //__nvic_set_priority(SVCall_IRQn, 0);
  vTaskStartScheduler();
init_systick();
END_SETUP

volatile uint64_t m;

//fail:
  usb_printf("\nERROR: Never can get here\n");
  for (;;) {

    iwd_refresh();
    m = milliseconds() + 500;
    ledon();

    while (m > milliseconds());

    iwd_refresh();
    m = milliseconds() + 500;
    ledoff();

    while (m > milliseconds());

  }
}

/*
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    usb_printf("\nTASK OVERFLOW\n\n");
    for (;;);
}
*/

#else

void run(void)
{
//volatile uint32_t tmp = SCB->AIRCR;
  usb_printf("\nInitializing ...\n\nPriority group: %u\n", get_priority_grouping());
//usb_printf("AIRCR before: 0x%08lX\n", tmp);

//__nvic_setprioritygrouping(NVIC_PRIORITYGROUP_4);
init_systick();
END_SETUP
  blink_n(3);
//usb_printf("AIRCR after:  0x%08lX\n", SCB->AIRCR);
  usb_printf("\nReady ...\n\n");
  while (1) {
    run_process();
    delay(1);
  }
}

#endif

static char text[1024];
size_t text_sz = 0;

void usb_receive(uint8_t *buf, uint32_t buf_sz)
{
  if ((size_t)buf_sz > sizeof(text)-1)
    text_sz = sizeof(text)-1;
  else
    text_sz = (size_t)buf_sz;

  text[text_sz] = 0;
  char *p = text;
  size_t tmp = text_sz;

  while (tmp > 0) {

    char c = *(buf++);
    if ((c != '\r') && (c != '\n'))
      *(p++) = c;
    else {
      *p = 0;
      break;
    }
 
    --tmp;
  }
}


void usb_receive_complete()
{
/*
  if (text_sz <= 2)
    return;
*/
  COMMAND_CHECK_CALL_ARG(ping)
  COMMAND_CHECK_CALL_ARG(meminfo)
  COMMAND_CHECK_CALL_ARG(timestamp)
  COMMAND_CHECK_CALL_ARG(setdate)
  COMMAND_CHECK_CALL_ARG(getdate)
  COMMAND_CHECK_CALL_ARG(help)
  COMMAND_CHECK_CALL_ARG(milliseconds)
  COMMAND_CHECK_CALL_ARG(cpuinfo)
  COMMAND_CHECK_CALL_ARG(sensors)

  usb_printf("Invalid command %.*s\n\n", text_sz, text);

}

void usb_error(int value)
{

  switch(value) {
    case E_USB_INIT:
      hasError = 7;
      break;
    case E_USB_REGISTER_CLASS:
      hasError = 6;
      break;
    case E_USB_REGISTER_INTERFACE:
      hasError = 4;
      break;
    case E_USB_START:
      hasError = 3;
      break;
    case E_USB_TRANSMIT_BUSY:
      hasError =1;
      break;
    case E_USB_TRANSMIT_FAIL:
      hasError = 2;
      break;
    case E_USB_HAL_PCD_HS:
      hasError = 8;
      break;
    default:
      hasError = 5;
  }

}

#ifdef RTOS_SOLAR48
void realtime(uint32_t time)
{
  usb_printf("\nTimestamp: %u\n HAS RTOS TICKS: %d\n", time, has_rtos_ticks());
}
#else

//TODO refactor. Testing
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
