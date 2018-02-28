#include "nRF5x_RTC.h"

void nRF5x_RTC_TIME_handler(uint8_t instance);

nRF5x_RTC_eventCallback alarmCallback;

nRF5x_RTC_eventCallback rtcCallback[RTC_INSTANCE_COUNT];

uint8_t nRF5x_RTC_inited[RTC_INSTANCE_COUNT];
uint8_t attachedCallback[RTC_INSTANCE_COUNT];
uint8_t enabledTime[RTC_INSTANCE_COUNT];
uint8_t tickerTime[RTC_INSTANCE_COUNT];

uint16_t nRF5x_TIME_startOffset = nRF5x_RTC_DEFAULT_START_YEAR;

nRF5x_RTC_TIME_t  TimeInternal  = nRF5x_RTC_DEFAULT_TIME;
nRF5x_RTC_DATE_t  DateInternal  = nRF5x_RTC_DEFAULT_DATE;
nRF5x_RTC_ALARM_t AlarmInternal = nRF5x_RTC_DEFAULT_ALARM;


// Получение экземпляра RTC по id
NRF_RTC_Type* nRF5x_RTC_getInstance(uint8_t instance) {
  switch(instance) {
    case RTC_INSTANCE_0: return NRF_RTC0;
    case RTC_INSTANCE_1: return NRF_RTC1;
    default: return NRF_RTC2;
  }
}

// Инициализация экземпляра RTC
void nRF5x_RTC_init(uint8_t instance, uint16_t prescaler) {
  if(instance >= RTC_INSTANCE_COUNT) return;
  if(nRF5x_RTC_inited[instance]) return;
  nrf_drv_clock_init();
  nrf_drv_clock_lfclk_request(NULL);
  // устанавливаем делитель частоты
  nrf_rtc_prescaler_set(nRF5x_RTC_getInstance(instance), prescaler);
  // активируем прерывания
  switch(instance) {
    case RTC_INSTANCE_0 : {
      NVIC_SetPriority(RTC0_IRQn, 2);
      NVIC_ClearPendingIRQ(RTC0_IRQn);
      NVIC_EnableIRQ(RTC0_IRQn);
    } break;
    case RTC_INSTANCE_1 : {
      NVIC_SetPriority(RTC1_IRQn, 2);
      NVIC_ClearPendingIRQ(RTC1_IRQn);
      NVIC_EnableIRQ(RTC1_IRQn);
    } break;
    case RTC_INSTANCE_2 : {
      NVIC_SetPriority(RTC2_IRQn, 2);
      NVIC_ClearPendingIRQ(RTC2_IRQn);
      NVIC_EnableIRQ(RTC2_IRQn);
    } break;
  }
  nrf_rtc_event_clear(nRF5x_RTC_getInstance(instance), NRF_RTC_EVENT_TICK);
  nrf_rtc_int_enable(nRF5x_RTC_getInstance(instance), NRF_RTC_INT_TICK_MASK);
  nRF5x_RTC_inited[instance] = true;
}

// Деинициализация экземпляра RTC
void nRF5x_RTC_deinit(uint8_t instance) {
  // необходимо отключать прерывания
}

// Подключение callback функции
void nRF5x_RTC_attachCallback(uint8_t instance, nRF5x_RTC_eventCallback function) {
  if(instance >= RTC_INSTANCE_COUNT) return;
  rtcCallback[instance] = function;
  attachedCallback[instance] = true;
}

// Отключение callback функции
void nRF5x_RTC_detachCallback(uint8_t instance) {
  if(instance >= RTC_INSTANCE_COUNT) return;
  rtcCallback[instance] = false;
  attachedCallback[instance] = false;
}

// Получение текущего значения счетчика
uint32_t nRF5x_RTC_get(uint8_t instance) {
  return nrf_rtc_counter_get(nRF5x_RTC_getInstance(instance));
}

// Запуск счетчика
void nRF5x_RTC_start(uint8_t instance) {
  nrf_rtc_task_trigger(nRF5x_RTC_getInstance(instance), NRF_RTC_TASK_START);
}

// Остановка счетчика
void nRF5x_RTC_stop(uint8_t instance) {
  nrf_rtc_task_trigger(nRF5x_RTC_getInstance(instance), NRF_RTC_TASK_STOP);
}

// Обработчик прерываний RTC0
void RTC0_IRQHandler(void) {
  if(nrf_rtc_event_pending(NRF_RTC0, NRF_RTC_EVENT_TICK)) {
    nrf_rtc_event_clear(NRF_RTC0, NRF_RTC_EVENT_TICK);
    // вызываем callback функцию если назначена
    if(attachedCallback[RTC_INSTANCE_0]) rtcCallback[RTC_INSTANCE_0]();
    if(enabledTime[RTC_INSTANCE_0]) nRF5x_RTC_TIME_handler(RTC_INSTANCE_0);
  }
}

// Обработчик прерываний RTC1
void RTC1_IRQHandler(void) {
  if(nrf_rtc_event_pending(NRF_RTC1, NRF_RTC_EVENT_TICK)) {
    nrf_rtc_event_clear(NRF_RTC1, NRF_RTC_EVENT_TICK);
    // вызываем callback функцию если назначена
    if(attachedCallback[RTC_INSTANCE_1]) rtcCallback[RTC_INSTANCE_1]();
    if(enabledTime[RTC_INSTANCE_1]) nRF5x_RTC_TIME_handler(RTC_INSTANCE_1);
  }
}

// Обработчик прерываний RTC2
void RTC2_IRQHandler(void) {
  if(nrf_rtc_event_pending(NRF_RTC2, NRF_RTC_EVENT_TICK)) {
    nrf_rtc_event_clear(NRF_RTC2, NRF_RTC_EVENT_TICK);
    // вызываем callback функцию если назначена
    if(attachedCallback[RTC_INSTANCE_2]) rtcCallback[RTC_INSTANCE_2]();
    if(enabledTime[RTC_INSTANCE_2]) nRF5x_RTC_TIME_handler(RTC_INSTANCE_2);
  }
}

//=============================================================================
void nRF5x_RTC_TIME_handler(uint8_t instance) {
  if(instance >= RTC_INSTANCE_COUNT) return;
  if(++tickerTime[instance]==8) {
    tickerTime[instance] = 0;
    if(++TimeInternal.second==60) {
      TimeInternal.second = 0;
      if(++TimeInternal.minute==60) {
        TimeInternal.minute = 0;
        if(++TimeInternal.hour==24) {
          TimeInternal.hour = 0;
          
        }
      }
    }
  }
  if(AlarmInternal.activated) {
    if(TimeInternal.second >= AlarmInternal.time.second) {
      AlarmInternal.activated = false;
      alarmCallback();
    }
  }
}

//=============================================================================
void nRF5x_RTC_enableTime(uint8_t instance) {
  if(instance >= RTC_INSTANCE_COUNT) return;
  if(enabledTime[instance]) return;
  enabledTime[instance] = true;
  nRF5x_RTC_init(instance, 4095);
  nRF5x_RTC_start(instance);
}

void nRF5x_RTC_disableTime(uint8_t instance) {
  if(instance >= RTC_INSTANCE_COUNT) return;
  enabledTime[instance] = false;
}

void nRF5x_RTC_resetTime(uint8_t instance) {
  nRF5x_RTC_setTimeValue(0, 0, 0);
  //nRF5x_RTC_setDateValue(1, 1, nRF5x_TIME_startOffset);
}

//=============================================================================

void nRF5x_RTC_setTime(nRF5x_RTC_TIME_t _time) {
  TimeInternal = _time;
}

void nRF5x_RTC_getTime(nRF5x_RTC_TIME_t* _time) {
  _time = &TimeInternal;
}

void nRF5x_RTC_setTimeValue(uint8_t _second, uint8_t _minute, uint8_t _hour) {
  TimeInternal.second = _second;
  TimeInternal.minute = _minute;
  TimeInternal.hour   = _hour;
}

void nRF5x_RTC_setSecond(uint8_t _second) { TimeInternal.second = _second; }
void nRF5x_RTC_setMinute(uint8_t _minute) { TimeInternal.minute = _minute; }
void nRF5x_RTC_setHour  (uint8_t _hour)   { TimeInternal.hour   = _hour;   }

uint8_t nRF5x_RTC_getSecond() { return TimeInternal.second; }
uint8_t nRF5x_RTC_getMinute() { return TimeInternal.minute; }
uint8_t nRF5x_RTC_getHour()   { return TimeInternal.hour;   }

//=============================================================================

void nRF5x_RTC_setAlarm(nRF5x_RTC_ALARM_t _alarm, nRF5x_RTC_eventCallback function) {
  AlarmInternal = _alarm;
  AlarmInternal.activated = true;
  alarmCallback = function;
}
