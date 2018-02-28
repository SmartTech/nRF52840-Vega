#ifndef _NRF5x_RTC_H_
#define _NRF5x_RTC_H_

#include "nrf_rtc.h"
#include "nrf_drv_clock.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
  RTC_INSTANCE_0=0,
  RTC_INSTANCE_1,
  RTC_INSTANCE_2,
  RTC_INSTANCE_COUNT
};

typedef struct {
  uint8_t  second;
  uint8_t  minute;
  uint8_t  hour;
} nRF5x_RTC_TIME_t;

typedef struct {
  uint8_t  day;
  uint8_t  month;
  uint16_t year;
} nRF5x_RTC_DATE_t;

typedef struct {
  nRF5x_RTC_TIME_t time;
  nRF5x_RTC_DATE_t date;
  uint8_t activated;
} nRF5x_RTC_ALARM_t;

#define nRF5x_RTC_DEFAULT_START_YEAR    2000

#define nRF5x_RTC_DEFAULT_TIME { \
        .second = 0,             \
        .minute = 0,             \
        .hour   = 0,           }

#define nRF5x_RTC_DEFAULT_DATE { \
        .day   = 1,              \
        .month = 1,              \
        .year  = nRF5x_RTC_DEFAULT_START_YEAR, }

#define nRF5x_RTC_DEFAULT_ALARM {       \
        .time = nRF5x_RTC_DEFAULT_TIME, \
        .date = nRF5x_RTC_DEFAULT_DATE, \
        .activated = false,            }

typedef void (*nRF5x_RTC_eventCallback)();

void     nRF5x_RTC_init(uint8_t instance, uint16_t prescaler);
void     nRF5x_RTC_deinit(uint8_t instance);
        
void     nRF5x_RTC_attachCallback(uint8_t instance, nRF5x_RTC_eventCallback function);
void     nRF5x_RTC_detachCallback(uint8_t instance);
        
void     nRF5x_RTC_start(uint8_t instance);
void     nRF5x_RTC_stop(uint8_t instance);

uint32_t nRF5x_RTC_get(uint8_t instance);

void     nRF5x_RTC_enableTime(uint8_t instance);
void     nRF5x_RTC_disableTime(uint8_t instance);
void     nRF5x_RTC_resetTime(uint8_t instance);

void     nRF5x_RTC_setTime(nRF5x_RTC_TIME_t _time);
void     nRF5x_RTC_getTime(nRF5x_RTC_TIME_t* _time);
void     nRF5x_RTC_setTimeValue(uint8_t _second, uint8_t _minute, uint8_t _hour);
void     nRF5x_RTC_setSecond(uint8_t _day);
void     nRF5x_RTC_setMinute(uint8_t _day);
void     nRF5x_RTC_setHour(uint8_t _day);
uint8_t  nRF5x_RTC_getSecond();
uint8_t  nRF5x_RTC_getMinute();
uint8_t  nRF5x_RTC_getHour();
         
void     nRF5x_RTC_setDate(nRF5x_RTC_DATE_t _time);
void     nRF5x_RTC_getDate(nRF5x_RTC_DATE_t* _time);
void     nRF5x_RTC_setDateValue(uint8_t _day, uint8_t _month, uint16_t _year);
void     nRF5x_RTC_setDay(uint8_t _day);
void     nRF5x_RTC_setMonth(uint8_t _month);
void     nRF5x_RTC_setYear(uint16_t _year);
uint8_t  nRF5x_RTC_getDay();
uint8_t  nRF5x_RTC_getMonth();
uint16_t nRF5x_RTC_getYear();

void     nRF5x_RTC_setAlarm(nRF5x_RTC_ALARM_t _alarm, nRF5x_RTC_eventCallback function);

#ifdef __cplusplus
}
#endif

#endif // _NRF5x_RTC_H_
