#include <registers.h>
#include <core_cm3.h>
#include <watchdog.h>
#include <systick_config.h>
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

//#define SYS_TICK_DIV     8// Use 8 if disable SysTick_CTRL_CLKSOURCE_Msk

#define SYS_TICK_DIV 1 // For FreeRTOS

/* FreeRTOS tick timer interrupt handler prototype */
extern void xPortSysTickHandler (void);

#define SYSTICK_TICKS    ((CPU_FREQ_HZ / (SYS_TICK_DIV * SYS_TICK_FREQ_HZ)) - 1UL)
//void init_systick()
//{
//  SysTick->LOAD  = (uint32_t)(SYSTICK_TICKS); /* set reload register */
//  NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
//  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
//  SysTick->CTRL  = /*SysTick_CTRL_CLKSOURCE_Msk |*/
//                   SysTick_CTRL_TICKINT_Msk   |
//                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

//}

void init_systick()
{
  SysTick->LOAD  = (uint32_t)(SYSTICK_TICKS); /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, configKERNEL_INTERRUPT_PRIORITY); /* set Priority for Systick Interrupt */
  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
  SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

}

static volatile uint64_t tick = 0;

void SysTick_Handler()
{
  //SysTick->CTRL;
  ++tick;

  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    /* Call tick handler */
    //iwd_refresh();
    xPortSysTickHandler();
  }
}

uint64_t milliseconds()
{
  return tick;
}
/*
uint64_t milliseconds()
{
  uint64_t t;
  __disable_irq();
  t = tick;
  __enable_irq();
  return t;
}
*/
void delay(uint64_t milliseconds)
{
  uint64_t lim = tick + milliseconds;

  while (tick < lim) {
    iwd_refresh();
  };
  //__WFI(); If only one interruption or principal
}

// TODO for test only. Delegate to RTOS timing system. Will be removed
static void delay_1us()
{
  asm volatile(
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
    "nop\n"
  );
}

//TODO for test only. Will be removed
void delay_5us()
{
  int i = 0;

  do
    delay_1us();
  while (++i < 5);
}
