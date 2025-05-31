#include <stddef.h>
#include <rtc.h>
#include <registers.h>
#include <stm32f103x6.h>
#include <core_cm3.h>


rtc_cb rtc_caller = NULL;

void init_rtc(rtc_cb rtc_callback)
{

  rtc_caller = rtc_callback;
  // RTC Clock configuration with 32,768 Hz external clock

  // Activate power interface clock enabled
  RCC_APB1ENR |= PWREN;

  //Disable backup domain write protection. Page 78
  PWR_CR |= DBP;

  // Enable external low-speed oscillator enable. Page 119
  RCC_BDCR |= LSEON;

  //Wait for external low-speed oscillator ready. Page 119
  while ((RCC_BDCR & LSERDY) == 0);

  // RTC clock source selection. Unselect and select the desired RTC clock. Page 118
  RCC_BDCR &= ~(RTC_SEL_mask);
  RCC_BDCR |= LSE_oscillator_clock_used_as_RTC_clock;
  RCC_BDCR |= RTCEN;

  // Wait hardware syncronization Registers synchronized flag. Page 488
  RTC_CRL &= ~RSF;
  while ((RTC_CRL & RSF) == 0);

  while ((RTC_CRL & RTOFF) == 0);
  // Begin configuration according to 18.3.84 configuration procedure step. Page 485
  RTC_CRL |= CNF;
  // Prescale 0x7FFF for 32768 Hz is exactly 1 second tick. Page 489
  RTC_PRLL = 0x7FFF;
  //Second interrupt enable. Page 487. According to hardware design Figure 179. RTC simplified block diagram. Page 484
  RTC_CRH |= SECIE;
  // End configuration RTC
  RTC_CRL &= ~CNF;
  while ((RTC_CRL & RTOFF) == 0);

  //Enable backup domain write protection. Page 78
  PWR_CR &= (~DBP);

  // Disable power interface clock enabled
  RCC_APB1ENR &= ~PWREN;

  // Enable RTC interrupt and its priority
  NVIC_EnableIRQ(RTC_IRQn);
  NVIC_SetPriority(RTC_IRQn, 10);
}

void RTC_IRQHandler(void)
{
  //SECF: Second flag set by hardware if prescale overflows. Set by hardware and . Page 
  if (RTC_CRL & SECF) {
    RTC_CRL &= ~SECF;
    if (rtc_caller) {
      while ((RTC_CRL & RSF) == 0);
      // Hardwware read counter and parsing to caller
      rtc_caller(RTC_CNTH<<16|RTC_CNTL);
      RTC_CRL &= ~RSF;
    }
  }
}

