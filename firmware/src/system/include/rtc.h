#ifndef RTC_H
 #define RTC_H
#include <stdint.h>

typedef void (*rtc_cb)(uint32_t);

void init_rtc(rtc_cb);
uint32_t rtc_get_timestamp();
#endif

