
extern void Default_Handler();

static const char *panic_irq = "Unknown IRQ call";

#define PANIC_IRQ(irq_fn) \
void irq_fn() \
{\
  panic_irq = #irq_fn;\
  Default_Handler();\
}

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

const char *irq_panic_info()
{
  return panic_irq;
}

