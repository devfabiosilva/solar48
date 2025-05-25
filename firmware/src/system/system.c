//ter 20 mai 2025 19:28:25 
#include <registers.h>

//#define TEST

#ifdef TEST
void system_init(void)
{
// TODO initialize clock system here
    RCC_APB2ENR |= IOPCEN;
}
#else

void system_init(void)
{
  RCC_CFGR = PLLMUL(PLLMULx9)|PPRE1(HCLK_div2)|PLLSRC;
  RCC_CR = HSEON|PLLON|CSSON;
  while ((RCC_CR & HSERDY)==0);
  while ((RCC_CR & PLLRDY)==0);
  RCC_CFGR |= SW(PLL_as_system_clock);
  while ((RCC_CFGR & SWS_mask) != PLL_selected_as_system_clock);
  RCC_APB2ENR |= IOPCEN;
  
  //USB
  //RCC_APB1ENR |= USBEN;
}

#endif
