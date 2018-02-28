#ifndef _NRF5x_TWI_H_
#define _NRF5x_TWI_H_

#include "nrf_drv_twi.h"
#include "nrf_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define nRF5x_TWI0        TWI0_INSTANCE_INDEX // 0
#define nRF5x_TWI1        TWI1_INSTANCE_INDEX // 1
  
//NRF_TWI_FREQ_100K
//NRF_TWI_FREQ_250K
//NRF_TWI_FREQ_400K
  
// Переопределение структуры конфигурации (для эстетики библиотеки) -----------
#define nRF5x_twi_config_t nrf_drv_twi_config_t
  
// Переопределение дефолтных значений конфигурации (для эстетики библиотеки) --
#define nRF5x_TWI_DEFAULT_CONFIG NRF_DRV_TWI_DEFAULT_CONFIG

// Инициализация экземпляра TWI с указанием основных пинов --------------------
uint8_t nRF5x_TWI_begin  (uint8_t instance, uint8_t _pinSDA, uint8_t _pinSCL);

// Функции переконфигурации экземпляра TWI ------------------------------------
uint8_t nRF5x_TWI_setFreq    (uint8_t instance, nrf_twi_frequency_t freq);
uint8_t nRF5x_TWI_setPriority(uint8_t instance, uint8_t priority);

// Инициализация экземпляра TWI по структуре конфигурации ---------------------
uint8_t nRF5x_TWI_init   (uint8_t instance, nRF5x_twi_config_t* config);

// Деинициализация экземпляра TWI ---------------------------------------------
uint8_t nRF5x_TWI_deinit (uint8_t instance);

uint8_t nRF5x_TWI_beginTransmission(uint8_t instance, uint8_t address);
uint8_t nRF5x_TWI_endTransmission(uint8_t instance);

uint8_t nRF5x_TWI_write(uint8_t instance, uint8_t* data, uint8_t size);
uint8_t nRF5x_TWI_read (uint8_t instance, uint8_t* data, uint8_t size);

uint8_t nRF5x_TWI_available(uint8_t instance);

uint8_t nRF5x_TWI_requastFrom(uint8_t instance, uint8_t address);

#ifdef __cplusplus
}
#endif

#endif // _NRF5x_TWI_H_