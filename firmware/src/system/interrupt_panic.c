#include <stdint.h>
#include <usb_io.h>
#include <system.h>
#include <time.h>
#include <watchdog.h>

#ifdef RTOS_SOLAR48

#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#endif

volatile static const char *panic_irq = "Unknown IRQ call";
void halt_ir();
/*
//https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-control-block
//arm-none-eabi-addr2line -e solar48_release.elf 0x08013528 -f -C

*/
void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{

  do {
    usb_printf(
      "\nr0=%08X\nr1=%08X\nr2=%08X\n",
      pulFaultStackAddress[ 0 ],
      pulFaultStackAddress[ 1 ],
      pulFaultStackAddress[ 2 ]
    );
  } while (0);


  do {
    usb_printf("r3=%08X\nr12=%08X\nlr=%08X\n",
      pulFaultStackAddress[ 3 ],
      pulFaultStackAddress[ 4 ],
      pulFaultStackAddress[ 5 ]
    );
  } while (0);

  do {
    usb_printf("pc=%08X\npsr=%08X\n",
      pulFaultStackAddress[ 6 ],
      pulFaultStackAddress[ 7 ]
    );
  } while (0);

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
  );\
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
PANIC_IRQ(DMA1_Channel4_IRQHandler)
PANIC_IRQ(DMA1_Channel5_IRQHandler)
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
PANIC_IRQ(I2C1_EV_IRQHandler)
PANIC_IRQ(I2C1_ER_IRQHandler)
PANIC_IRQ(SPI1_IRQHandler)
PANIC_IRQ(USART1_IRQHandler)
PANIC_IRQ(USART2_IRQHandler)
PANIC_IRQ(EXTI15_10_IRQHandler)
PANIC_IRQ(RTC_Alarm_IRQHandler)
PANIC_IRQ(USBWakeUp_IRQHandler)

#define SOLAR48_PANIC for (;;) {\
    iwd_refresh();\
    __asm("nop");\
    __asm("nop");\
  }

void halt()
{
  // It could not happen. If happens report bug and disable all interrupts
  usb_printf("\nHALT\n\n");
  load_panic_stack();
  DISABLE_SETUP
  SOLAR48_PANIC
}

void halt_ir()
{
  // It could not happen. If happens report bug and disable all interrupts
  usb_printf("\nHALT IRQ = %s\n\n", panic_irq);
  load_panic_stack();
  DISABLE_SETUP
  SOLAR48_PANIC
}

#ifdef RTOS_SOLAR48
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    // Trap here for debug
    usb_printf("\nTask = %p and task name %s\n\n", xTask, pcTaskName);
    halt();
}

void vApplicationMallocFailedHook(void)
{
    // Malloc failed trap
    usb_printf("\nFailed malloc\n");
    halt();
}
#endif

