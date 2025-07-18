#include <registers.h>
#include <core_cm3.h>
#include <watchdog.h>
#include <systick_config.h>

//#define SYSTICK_TICKS    ((CPU_FREQ_HZ / (SYS_TICK_DIV * SYS_TICK_FREQ_HZ)) - 1UL)

#ifdef RTOS_SOLAR48
// RTOS in Solar48
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>

/* FreeRTOS tick timer interrupt handler prototype */
extern void xPortSysTickHandler (void);

void vPortSetupTimerInterrupt()
{

  SysTick->LOAD  = (uint32_t)(SYSTICK_TICKS); /* set reload register */
  //__nvic_set_priority(SysTick_IRQn, SYSTICK_PRIO);
  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
  SysTick->CTRL  = /*SysTick_CTRL_CLKSOURCE_Msk | */ // Clock div 8 if commented
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

}

#else
// Default System in Solar48

#include <sys_interrupts.h>
//#define SYS_TICK_DIV     8// Use 8 if disable SysTick_CTRL_CLKSOURCE_Msk

void init_systick()
{
  SysTick->LOAD  = (uint32_t)(SYSTICK_TICKS); /* set reload register */
  __nvic_set_priority(SysTick_IRQn, SYSTICK_PRIO);
  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
  SysTick->CTRL  = /*SysTick_CTRL_CLKSOURCE_Msk |*/ // Clock div 8 if commented
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

}

#endif

static volatile uint64_t tick = 0;
static volatile int rtos_tick = 0;

void SysTick_Handler()
{
  ++tick;

#ifdef RTOS_SOLAR48
//  SysTick->CTRL;
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
    // Call tick handler
     xPortSysTickHandler();
     rtos_tick = 1;
  } else
    rtos_tick = 0;
#endif
}

uint64_t milliseconds()
{
  return (volatile uint64_t)tick;
}

#ifdef RTOS_SOLAR48
int has_rtos_ticks()
{
  return rtos_tick;
}

#else
void delay(uint64_t milliseconds)
{
  uint64_t lim = tick + milliseconds;

  while (tick < lim) {
    iwd_refresh();
  };
  //__WFI(); If only one interruption or principal
}
#endif

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

