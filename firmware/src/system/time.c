#include <registers.h>
#include <core_cm3.h>
#include <watchdog.h>

#define SYS_TICK_FREQ_HZ 1000UL  // 1ms
#define CPU_FREQ_HZ      72000000UL
#define SYS_TICK_DIV     8// Use 8 if disable SysTick_CTRL_CLKSOURCE_Msk

#define SYSTICK_TICKS    (CPU_FREQ_HZ / (SYS_TICK_DIV * SYS_TICK_FREQ_HZ)) - 1UL
void init_systick()
{
  SysTick->LOAD  = (uint32_t)(SYSTICK_TICKS); /* set reload register */
  NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL); /* set Priority for Systick Interrupt */
  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
  SysTick->CTRL  = /*SysTick_CTRL_CLKSOURCE_Msk |*/
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

}

volatile uint64_t tick = 0;

void SysTick_Handler()
{
  ++tick;
}

uint64_t milliseconds()
{
  return tick;
}

void delay(uint64_t milliseconds)
{
  uint64_t lim = tick + milliseconds;

  while (tick < lim) {
    iwd_refresh();
  };
  //__WFI(); If only one interruption or principal
}

