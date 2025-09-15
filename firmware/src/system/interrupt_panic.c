#include <stdint.h>
#include <system.h>
#include <time.h>
#include <watchdog.h>
#include <solar48_config.h> 

#ifdef USE_USB_PRINTF_ON_PANIC
 #include <usb_io.h>
 #include <registers.h>

extern int cdc_transmit_is_busy();

static void panic_wait()
{
  uint32_t current_time = CURRENT_TIMESTAMP;

  while (cdc_transmit_is_busy()) {
    iwd_refresh();
    if (current_time != CURRENT_TIMESTAMP)
      break;
  }
}

#else
 #include <peripheral/ssd1306/oled_utils.h>
 extern volatile bool oled_util_lock;
 extern volatile bool update_screen_idle;
 extern volatile bool oled_buffer_idle;

 #define OLED_PANIC_PREPARE \
   oled_util_lock = true; \
   update_screen_idle = false; \
   oled_buffer_idle = false; \
   ssd1306_Fill(Black); \
   ssd1306_SetCursor(0, 0);

#endif

#ifdef RTOS_SOLAR48

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#endif

volatile static const char *panic_irq = "Unknown IRQ call";
void halt_ir();

void app_panic(const char *app_name)
{
  panic_irq = app_name;
  halt_ir();
}

/*
//https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-control-block
//arm-none-eabi-addr2line -e solar48_release.elf 0x08013528 -f -C

*/
void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{
#ifdef USE_USB_PRINTF_ON_PANIC
  panic_wait();

  usb_printf(
    "\nr0=%08X\nr1=%08X\nr2=%08X\nr3=%08X\nr12=%08X\nlr=%08X\npc=%08X\npsr=%08X\n",
    pulFaultStackAddress[ 0 ],
    pulFaultStackAddress[ 1 ],
    pulFaultStackAddress[ 2 ],
    pulFaultStackAddress[ 3 ],
    pulFaultStackAddress[ 4 ],
    pulFaultStackAddress[ 5 ],
    pulFaultStackAddress[ 6 ],
    pulFaultStackAddress[ 7 ]
  );

  panic_wait();
#else
  _oled_printf_panic(
    "r0=%08x\nr1=%08x\nr2=%08x\nr3=%08x\nr12=%08x\nlr=%08x\npc=%08x\npsr=%08x\n",
    pulFaultStackAddress[ 0 ],
    pulFaultStackAddress[ 1 ],
    pulFaultStackAddress[ 2 ],
    pulFaultStackAddress[ 3 ],
    pulFaultStackAddress[ 4 ],
    pulFaultStackAddress[ 5 ],
    pulFaultStackAddress[ 6 ],
    pulFaultStackAddress[ 7 ]
  );
#endif
}

void load_panic_stack()
{
  __asm volatile(
    "tst lr, #4\n"
    "ite eq\n"
    "mrseq r0, msp\n"
    "mrsne r0, psp\n"
    "ldr r1, [r0, #24]\n"
    "ldr r2, handler2_address_const\n"
    "bx r2\n"
    "handler2_address_const: .word prvGetRegistersFromStack\n"
  );
}

#define PANIC_IRQ(irq_fn) \
void irq_fn() \
{\
  panic_irq = #irq_fn;\
  halt_ir();\
}

//https://www.freertos.org/Documentation/02-Kernel/03-Supported-devices/04-Demos/Others/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers
PANIC_IRQ(NMI_Handler)
PANIC_IRQ(HardFault_Handler)
PANIC_IRQ(MemManage_Handler)
PANIC_IRQ(BusFault_Handler)
PANIC_IRQ(UsageFault_Handler)

#ifndef RTOS_SOLAR48
PANIC_IRQ(SVC_Handler)
#endif

PANIC_IRQ(DebugMon_Handler)

#ifndef RTOS_SOLAR48
PANIC_IRQ(PendSV_Handler)
#endif

//PANIC_IRQ(SysTick_Handler)
PANIC_IRQ(WWDG_IRQHandler)
PANIC_IRQ(PVD_IRQHandler)
PANIC_IRQ(TAMPER_IRQHandler)
//PANIC_IRQ(RTC_IRQHandler)
PANIC_IRQ(FLASH_IRQHandler)
PANIC_IRQ(RCC_IRQHandler)
PANIC_IRQ(EXTI0_IRQHandler)
PANIC_IRQ(EXTI1_IRQHandler)
PANIC_IRQ(EXTI2_IRQHandler)
PANIC_IRQ(EXTI3_IRQHandler)
PANIC_IRQ(EXTI4_IRQHandler)
PANIC_IRQ(DMA1_Channel1_IRQHandler)
PANIC_IRQ(DMA1_Channel2_IRQHandler)
PANIC_IRQ(DMA1_Channel3_IRQHandler)
//PANIC_IRQ(DMA1_Channel4_IRQHandler)
//PANIC_IRQ(DMA1_Channel5_IRQHandler)
PANIC_IRQ(DMA1_Channel6_IRQHandler)
PANIC_IRQ(DMA1_Channel7_IRQHandler)
PANIC_IRQ(ADC1_2_IRQHandler)
PANIC_IRQ(USB_HP_CAN1_TX_IRQHandler)
//PANIC_IRQ(USB_LP_CAN1_RX0_IRQHandler)
PANIC_IRQ(CAN1_RX1_IRQHandler)
PANIC_IRQ(CAN1_SCE_IRQHandler)
PANIC_IRQ(EXTI9_5_IRQHandler)
PANIC_IRQ(TIM1_BRK_IRQHandler)
PANIC_IRQ(TIM1_UP_IRQHandler)
PANIC_IRQ(TIM1_TRG_COM_IRQHandler)
PANIC_IRQ(TIM1_CC_IRQHandler)
PANIC_IRQ(TIM2_IRQHandler)
PANIC_IRQ(TIM3_IRQHandler)
PANIC_IRQ(TIM4_IRQHandler)
PANIC_IRQ(I2C1_EV_IRQHandler)
PANIC_IRQ(I2C1_ER_IRQHandler)
PANIC_IRQ(I2C2_EV_IRQHandler)
PANIC_IRQ(I2C2_ER_IRQHandler)
PANIC_IRQ(SPI1_IRQHandler)
PANIC_IRQ(SPI2_IRQHandler)
//PANIC_IRQ(USART1_IRQHandler)
PANIC_IRQ(USART2_IRQHandler)
PANIC_IRQ(USART3_IRQHandler)
PANIC_IRQ(EXTI15_10_IRQHandler)
PANIC_IRQ(RTC_Alarm_IRQHandler)
PANIC_IRQ(USBWakeUp_IRQHandler)

#define SOLAR48_PANIC for (;;) {\
    __asm("nop");\
    __asm("nop");\
    iwd_refresh();\
    __asm("nop");\
    __asm("nop");\
  }

void halt()
{
  // It could not happen. If happens report bug and disable all interrupts
#ifdef USE_USB_PRINTF_ON_PANIC
  panic_wait();
  usb_printf("\nHALT\n\n");
#else
  OLED_PANIC_PREPARE
  _oled_printf_panic("HALT");
  ssd1306_SetCursor(0, 12);
#endif

  load_panic_stack();
  DISABLE_SETUP
  SOLAR48_PANIC
}

void halt_ir()
{
  // It could not happen. If happens report bug and disable all interrupts

#ifdef USE_USB_PRINTF_ON_PANIC
  panic_wait();
  usb_printf("\nHALT IRQ = %s\n\n", panic_irq);
#else
  OLED_PANIC_PREPARE
  _oled_printf_panic("HALT IRQ = %s", panic_irq);
  ssd1306_SetCursor(0, 12);
#endif

  load_panic_stack();
  DISABLE_SETUP
  SOLAR48_PANIC
}

#ifdef RTOS_SOLAR48
static void halt_rtos()
{
  load_panic_stack();
  DISABLE_SETUP
  SOLAR48_PANIC
}
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // Trap here for debug
#ifdef USE_USB_PRINTF_ON_PANIC
  panic_wait();
  usb_printf("\nTask = %p halted: %s\n\n", xTask, pcTaskName);
#else
  OLED_PANIC_PREPARE
  _oled_printf_panic("Task = %p halted: %s", xTask, pcTaskName);
  ssd1306_SetCursor(0, 12);
#endif
  halt_rtos();
}

void vApplicationMallocFailedHook(void)
{
    // Malloc failed trap
#ifdef USE_USB_PRINTF_ON_PANIC
  panic_wait();
  usb_printf("\nFailed malloc\n");
#else
  OLED_PANIC_PREPARE
  _oled_printf_panic("Failed malloc");
  ssd1306_SetCursor(0, 12);
#endif
  halt_rtos();
}
#endif

