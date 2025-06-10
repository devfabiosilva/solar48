#include <registers.h>
#include <stdbool.h>

bool iwd_fault()
{
  return (RCC_CSR & IWDGRSTF) != 0;
}

void iwd_refresh()
{
  //These bits must be written by software at regular intervals with the key value AAAAh. Page 496
  IWDG_KR = 0xAAAA;
}

void reset_wdg_fault()
{
  RCC_CSR |= RMVF;
}

void init_idw()
{
/*
  // Initialize Internal Low Speed clock 40 kHz. Page 119
  RCC_CSR |= LSION;
  while ((RCC_CSR & LSIRDY) == 0);
*/
  // Writing the key value 5555h to enable access to the IWDG_PR and IWDG_RLR registers. Page 496
  IWDG_KR = 0x5555;

  // Prescale division. Page 497
  IWDG_PR = PR_divide_by_256;

  //Timeout = (Reload + 1) / (LSI / Prescaler) ~ 1s
  IWDG_RLR = 156; // ~1 second

  //These bits must be written by software at regular intervals with the key value AAAAh. Page 496
  IWDG_KR = 0xAAAA;

  //Writing the key value CCCCh starts the watchdog. Page 496
  IWDG_KR = 0xCCCC;
}

void WWDG_IRQHandler()
{
// TODO add event

}

