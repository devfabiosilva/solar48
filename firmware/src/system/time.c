#include <registers.h>
#include <core_cm3.h>
#include <watchdog.h>

//#define SYSTICK_TICKS    ((CPU_FREQ_HZ / (SYS_TICK_DIV * SYS_TICK_FREQ_HZ)) - 1UL)

static volatile uint64_t tick = 0;

#ifdef RTOS_SOLAR48
// RTOS in Solar48
#include <FreeRTOS/FreeRTOS.h>
#include <FreeRTOS/task.h>
#include <time.h>

/* FreeRTOS tick timer interrupt handler prototype */
extern void xPortSysTickHandler (void);

void rtos_milli_task(void *param)
{

  (void)param;

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1));
    ++tick;
  }
}

void vPortSetupTimerInterrupt()
{

  SysTick->LOAD  = (uint32_t)(SYSTICK_TICKS); /* set reload register */
  //__nvic_set_priority(SysTick_IRQn, SYSTICK_PRIO);
  SysTick->VAL   = 0UL;                                             /* Load the SysTick Counter Value */
  SysTick->CTRL  = /*SysTick_CTRL_CLKSOURCE_Msk | */ // Clock div 8 if commented
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                         /* Enable SysTick IRQ and SysTick Timer */

}

#endif
//#else
// Default System in Solar48

#include <solar48_config.h>
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

//#endif

void SysTick_Handler()
{

#ifdef RTOS_SOLAR48
//  SysTick->CTRL;
// Call tick handler
  if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
     xPortSysTickHandler();
  else
#endif
    ++tick;
}

uint64_t milliseconds()
{
  return (volatile uint64_t)tick;
}

#ifndef RTOS_SOLAR48

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

