//ter 20 mai 2025 19:28:25 
#include <registers.h>
#include <memory.h>
#include <core_cm3.h>
#include <watchdog.h>
#include <time.h>
#include <stdbool.h>

extern void app_panic(const char *);

void system_init(void)
{
  // Clock configurations (72 MHz with 8 MHz external clock)
  RCC_CFGR = PLLMUL(PLLMULx9)|PPRE1(HCLK_div2)|PLLSRC;
  RCC_CR = HSEON|PLLON|CSSON;
  while ((RCC_CR & HSERDY)==0);
  while ((RCC_CR & PLLRDY)==0);
  RCC_CFGR |= SW(PLL_as_system_clock);
  while ((RCC_CFGR & SWS_mask) != PLL_selected_as_system_clock);
//  RCC_APB2ENR |= IOPCEN;

  SCB->VTOR = 0x08000000;
  fill_stack_with_pattern();
}

bool sys_try_lock(volatile bool *lock, TIMEOUT_MS *timeout_ms, uint32_t wait, const char *message_on_panic)
{
  init_timeout_ms(timeout_ms, wait);

  do {
    bool expected = false;

    // See https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    if (__atomic_compare_exchange_n(lock, &expected, true, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
      return true;

    if (is_timeout_ms(timeout_ms)) {
      if (message_on_panic)
        app_panic(message_on_panic);

      return false;
    }

    iwd_refresh();
  } while (1);
}

bool sys_try_lock_if_gbl_is_false(volatile bool *global_lock, volatile bool *lock, TIMEOUT_MS *timeout_ms, uint32_t wait, const char *message_on_panic)
{
  init_timeout_ms(timeout_ms, wait);

  do {

    if (__atomic_load_n(global_lock, __ATOMIC_SEQ_CST))
      return false;

    bool expected = false;

    // See https://gcc.gnu.org/onlinedocs/gcc/_005f_005fatomic-Builtins.html
    if (__atomic_compare_exchange_n(lock, &expected, true, false, __ATOMIC_ACQUIRE, __ATOMIC_RELAXED))
      return true;

    if (is_timeout_ms(timeout_ms)) {
      if (message_on_panic)
        app_panic(message_on_panic);

      return false;
    }

    iwd_refresh();
  } while (1);
}

void sys_unlock(volatile bool *lock) {
  __atomic_store_n(lock, false, __ATOMIC_RELEASE);
}

