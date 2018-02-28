#ifndef _NRF5x_SPI_H_
#define _NRF5x_SPI_H_

#define nRF5x_SPI_VERSION   1

#include "types.h"
#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_drv_spis.h"

#define nRF5x_SPI0        SPI0_INSTANCE_INDEX // 0
#define nRF5x_SPI1        SPI1_INSTANCE_INDEX // 1
#define nRF5x_SPI2        SPI2_INSTANCE_INDEX // 2
#define nRF5x_SPI3        SPI3_INSTANCE_INDEX // 3


#ifdef __cplusplus
extern "C" {
#endif

// Переопределение структуры конфигурации (для эстетики библиотеки) -----------
#define nRF5x_spi_config_t nrf_drv_spi_config_t
  
// Переопределение дефолтных значений конфигурации (для эстетики библиотеки) --
#define nRF5x_SPI_DEFAULT_CONFIG NRF_DRV_SPI_DEFAULT_CONFIG
  
// Инициализация экземпляра SPI с указанием основных пинов --------------------
uint8_t nRF5x_SPI_begin      (uint8_t instance, uint8_t mosi, uint8_t miso, uint8_t sck);

// Функции переконфигурации экземпляра SPI ------------------------------------
uint8_t nRF5x_SPI_setMode    (uint8_t instance, uint8_t mode);
uint8_t nRF5x_SPI_setBitOrder(uint8_t instance, uint8_t bitOrder);
uint8_t nRF5x_SPI_setNSS     (uint8_t instance, uint8_t pinNSS);
uint8_t nRF5x_SPI_setFreq    (uint8_t instance, nrf_drv_spi_frequency_t freq);
  
// Инициализация экземпляра SPI по структуре конфигурации ---------------------
uint8_t nRF5x_SPI_init(uint8_t instance, nRF5x_spi_config_t* config);

// Деинициализация экземпляра SPI ---------------------------------------------
uint8_t nRF5x_SPI_deinit (uint8_t instance);

// Передача данных по SPI -----------------------------------------------------
uint8_t nRF5x_SPI_writeRead(uint8_t instance,
                            uint8_t* dataTX, uint8_t  sizeTX,
                            uint8_t* dataRX, uint8_t  sizeRX);

// Передача байта по SPI и получение ответного байта --------------------------
uint8_t nRF5x_SPI_transfer(uint8_t instance, uint8_t byteTX);

#ifdef __cplusplus
}
#endif

#endif // _NRF5x_SPI_H_