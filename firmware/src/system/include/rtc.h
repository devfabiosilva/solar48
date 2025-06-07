#ifndef RTC_H
 #define RTC_H
#include <stdint.h>
#include <stdbool.h>

typedef void (*rtc_cb)(uint32_t);

typedef struct solar48_date_t {
  uint16_t year; // Year since 1970
  uint8_t month; // Month 1 for January .. 12 for December
  uint8_t day; // Day 1 to 31
  uint8_t hour; // 0 to 23
  uint8_t minute; // 0 to 59
  uint8_t second; // 0 to 59
  uint8_t week_day; // day of the week 0 => Sun 6 => Sat
} SOLAR48_DATE;

// Initializer RTC HAL
void init_rtc(rtc_cb);

// Timestamp get and setter
uint32_t rtc_get_timestamp();
void rtc_set_timestamp(uint32_t);
void get_solar48_date(SOLAR48_DATE *, uint32_t *);
bool set_date(SOLAR48_DATE *);
const char *get_day_str(int, int, int);
const char *get_day_str1(int);
int get_day(int, int, int);

// Date/Time validation
bool is_leap_year(unsigned int);
bool valid_time(uint8_t, uint8_t, uint8_t);
bool valid_timestamp(int64_t);

#endif

