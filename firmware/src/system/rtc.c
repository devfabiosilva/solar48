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

#define CURRENT_TIMESTAMP RTC_CNTH<<16|RTC_CNTL
void RTC_IRQHandler(void)
{
  //SECF: Second flag set by hardware if prescale overflows. Set by hardware and . Page 
  if (RTC_CRL & SECF) {
    RTC_CRL &= ~SECF;
    if (rtc_caller) {
      while ((RTC_CRL & RSF) == 0);
      // Hardwware read counter and parsing to caller
      rtc_caller(CURRENT_TIMESTAMP);
    }
    RTC_CRL &= ~RSF;
  }
}

uint32_t old_current_time = 0;

uint32_t rtc_get_timestamp()
{
  if (RTC_CRL & RSF)
    return old_current_time = CURRENT_TIMESTAMP;

  return old_current_time;
}

void rtc_set_timestamp(uint32_t time)
{
  // Activate power interface clock enabled
  RCC_APB1ENR |= PWREN;

  //Disable backup domain write protection. Page 78
  PWR_CR |= DBP;

  while ((RTC_CRL & RTOFF) == 0);
  // Begin configuration according to 18.3.84 configuration procedure step. Page 485
  RTC_CRL |= CNF;
  //Second interrupt disable. Page 487. According to hardware design Figure 179. RTC simplified block diagram. Page 484
  RTC_CRH &= ~SECIE;

  RTC_CNTL = (uint16_t)(time & 0xFFFF);
  RTC_CNTH = (uint16_t)(time >> 16);

  // Second interrupt enable. Page 487. According to hardware design Figure 179. RTC simplified block diagram. Page 484
  RTC_CRH |= SECIE;

  // End configuration RTC
  RTC_CRL &= ~CNF;
  while ((RTC_CRL & RTOFF) == 0);

  //Enable backup domain write protection. Page 78
  PWR_CR &= (~DBP);

  // Disable power interface clock enabled
  RCC_APB1ENR &= ~PWREN;

}

// 0 is common year, 1 is leap year
unsigned char is_leap_year(unsigned int year)
{
  return (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
}

static const unsigned char days_in_month[] = {
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

// Return day if valid, or 0 if invalid date or -1 if unable to calculate
static char valid_date(unsigned int year, unsigned char month, unsigned char day)
{
  if (month < 1 || month > 12 || year < 1753 )
    return -1;

  unsigned char max_day = days_in_month[month - 1];

  if (month == 2 && is_leap_year(year))
    max_day = 29;

  return (day <= max_day) ? day : 0;
}

static int get_day(int y, int m, int d)
{
  //Sakamoto's formula
  return (d += m < 3 ? y-- : y - 2 , 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
}

static const char *weekday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "??"};

static const char *get_day_str(int y, int m, int d)
{
  return (const char *)weekday[get_day(y, m, d) & 0x07];
}

#define MINUTES_IN_SECONDS (uint32_t)(60)
#define HOURS_IN_SECONDS (uint32_t)(60*MINUTES_IN_SECONDS)
#define DAYS_IN_SECONDS (uint32_t)(24*HOURS_IN_SECONDS)
#define YEARS_IN_SECONDS (uint32_t)(365*DAYS_IN_SECONDS)

#define DAY_JAN (uint32_t)31
#define DAY_FEB (DAY_JAN + 28)
#define DAY_MAR (DAY_FEB + 31)
#define DAY_APR (DAY_MAR + 30)
#define DAY_MAY (DAY_APR + 31)
#define DAY_JUN (DAY_MAY + 30)
#define DAY_JUL (DAY_JUN + 31)
#define DAY_AUG (DAY_JUL + 31)
#define DAY_SEP (DAY_AUG + 30)
#define DAY_OCT (DAY_SEP + 31)
#define DAY_NOV (DAY_OCT + 30)
#define DAY_DEC (DAY_NOV + 31)

static uint32_t month_table[] = {
 DAY_JAN, DAY_FEB, DAY_MAR, DAY_APR,
 DAY_MAY, DAY_JUN, DAY_JUL, DAY_AUG,
 DAY_SEP, DAY_OCT, DAY_NOV, DAY_DEC
};

void get_solar48_local_date(SOLAR48_DATE *sd)
{
  uint32_t timestamp = rtc_get_timestamp();

  sd->year = (uint16_t)((uint32_t)(timestamp / YEARS_IN_SECONDS)) + 1970;

  uint32_t seconds_left_in_year = timestamp % YEARS_IN_SECONDS;

  uint32_t days = (seconds_left_in_year / DAYS_IN_SECONDS);
  uint32_t past_seconds_in_one_day;

  if (month_table[0] >= days) {
    sd->month = 1;
    sd->day = days;

    goto get_solar48_local_date_resume1;
  }

  uint32_t special_case = month_table[1];

  if ((special_case >= days) || ((special_case + 1 == days) && is_leap_year((unsigned int )sd->year))) {
    sd->month = 2;
    sd->day = (days - month_table[0]);
    goto get_solar48_local_date_resume1;
  }

  uint8_t m = 2;
  do {
    if (month_table[(size_t)m++] >= days)
      break;
  } while ((size_t)m < sizeof(month_table) / sizeof(month_table[0]));

  sd->month = m;
  sd->day = days - month_table[m - 2];

get_solar48_local_date_resume1:

  if (!sd->day)
    sd->day = 1;

  past_seconds_in_one_day = (seconds_left_in_year % DAYS_IN_SECONDS);
  uint32_t past_hour_in_one_day = (past_seconds_in_one_day / HOURS_IN_SECONDS);
  uint32_t past_second_in_one_hour = (past_seconds_in_one_day % HOURS_IN_SECONDS);
  uint32_t past_minutes_in_one_hour = past_second_in_one_hour / 60;

  sd->hour = (uint8_t)past_hour_in_one_day;
  sd->minute = (uint8_t)past_minutes_in_one_hour;
  sd->second = (uint8_t)((uint32_t)(past_second_in_one_hour % 60));

  sd->week_day = (uint8_t)get_day((unsigned int)sd->year, sd->month, sd->day);
}
#undef CURRENT_TIMESTAMP

