#include <stdint.h>
#include <usb_io.h>
#include <system.h>
#include <time.h>

#ifdef RTOS_SOLAR48
//TODO remove
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

#endif

extern void Default_Handler();

volatile static const char *panic_irq = "Unknown IRQ call";

#define PANIC_IRQ(irq_fn) \
void irq_fn() \
{\
  panic_irq = #irq_fn;\
  Default_Handler();\
}
/*
r0=20005000
r1=20000348
r2=E0E00000
r3=E000E000
r12=0800E48F
lr=FFFFFFF9
pc=0800C500
psr=0000000B

r0=A5A5A5A5
r1=20000348
r2=E0E00000
r3=E000E000
r12=0800E49F
lr=08009B23
pc=C8C0845C
psr=00000000
//https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-control-block
//arm-none-eabi-addr2line -e solar48_release.elf 0x0800C500 -f -C

*/
void prvGetRegistersFromStack( uint32_t *pulFaultStackAddress )
{

/*
  r0 = pulFaultStackAddress[ 0 ];
  r1 = pulFaultStackAddress[ 1 ];
  r2 = pulFaultStackAddress[ 2 ];
  r3 = pulFaultStackAddress[ 3 ];
  r12 = pulFaultStackAddress[ 4 ];
  lr = pulFaultStackAddress[ 5 ];
  pc = pulFaultStackAddress[ 6 ];
  psr = pulFaultStackAddress[ 7 ];
*/
/*
  do {
    usb_printf(
      "\nr0=%08X\nr1=%08X\nr2=%08X\n",
      pulFaultStackAddress[ 0 ],
      pulFaultStackAddress[ 1 ],
      pulFaultStackAddress[ 2 ]
    );
  } while (0);
*/
/*
  do {
    usb_printf("r3=%08X\nr12=%08X\nlr=%08X\n",
      pulFaultStackAddress[ 3 ],
      pulFaultStackAddress[ 4 ],
      pulFaultStackAddress[ 5 ]
    );
  } while (0);
*/
  do {
    usb_printf("pc=%08X\npsr=%08X\n",
      pulFaultStackAddress[ 6 ],
      pulFaultStackAddress[ 7 ]
    );
  } while (0);

  Default_Handler();
}


//https://www.freertos.org/Documentation/02-Kernel/03-Supported-devices/04-Demos/Others/Debugging-Hard-Faults-On-Cortex-M-Microcontrollers
PANIC_IRQ(NMI_Handler)
//PANIC_IRQ(HardFault_Handler)

void HardFault_Handler(void)
{
  panic_irq = "HardFault_Handler";

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

PANIC_IRQ(MemManage_Handler)
PANIC_IRQ(BusFault_Handler)
PANIC_IRQ(UsageFault_Handler)

#ifndef RTOS_SOLAR48
PANIC_IRQ(SVC_Handler)
#else
/*
extern void *pxCurrentTCB;


__attribute__((naked)) void SVC_Handler(void)
{
  __asm volatile(
    "ldr r3, =pxCurrentTCB      \n"  // r3 <- address pxCurrentTCB
    "ldr r2, [r3]               \n"  // r2 <- *pxCurrentTCB
    "cbz r2, no_sched_started   \n"  // if NULL, jump
    "b vPortSVCHandler          \n"  // if not null call
    "no_sched_started:          \n"
    "bx lr                      \n"  // return
  );

}
*/
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

void halt()
{
  // It could not happen. If happens report bug and disable all interrupts
  usb_printf("\nHALT\n\n");
  DISABLE_SETUP
}

void halt_ir()
{
  // It could not happen. If happens report bug and disable all interrupts
  usb_printf("\nHALT IRQ = %s\n\n", panic_irq);
  DISABLE_SETUP
}

#ifdef RTOS_SOLAR48
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    // Trap here for debug
    usb_printf("\nTask = %p and task name %s\n\n", xTask, pcTaskName);
    while (1);
}

void vApplicationMallocFailedHook(void)
{
    // Malloc failed trap
    usb_printf("\nFailed malloc\n");
    while (1);
}
#endif
