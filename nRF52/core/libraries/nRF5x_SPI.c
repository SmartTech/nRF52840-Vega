#include "nRF5x_SPI.h"

//-----------------------------------------------------------------------------
// Флгаи инициализации экземпляров UART
uint8_t nRF5x_SPI_inited_flag[SPI_INSTANCE_COUNT] = {false, false, false, false};

static volatile bool spi_xfer_done[SPI_INSTANCE_COUNT];

nRF5x_spi_config_t nRF5x_spi_config[SPI_INSTANCE_COUNT] = {
                                                    nRF5x_SPI_DEFAULT_CONFIG,
                                                    nRF5x_SPI_DEFAULT_CONFIG,
                                                    nRF5x_SPI_DEFAULT_CONFIG,
                                                    nRF5x_SPI_DEFAULT_CONFIG};

//-----------------------------------------------------------------------------
// Обработчик приема данных по SPI
void nRF5x_SPI_handler(uint8_t instance, nrf_drv_spi_evt_t const * p_event) {
  spi_xfer_done[instance] = true;
}

//-----------------------------------------------------------------------------
// Обработчик приема данных экземпляра SPI0
void nRF5x_SPI_0_handler(nrf_drv_spi_evt_t const * p_event, void* p_context) {
  nRF5x_SPI_handler(nRF5x_SPI0, p_event);
}
// Обработчик приема данных экземпляра SPI1
void nRF5x_SPI_1_handler(nrf_drv_spi_evt_t const * p_event, void* p_context) {
  nRF5x_SPI_handler(nRF5x_SPI1, p_event);
}
// Обработчик приема данных экземпляра SPI2
void nRF5x_SPI_2_handler(nrf_drv_spi_evt_t const * p_event, void* p_context) {
  nRF5x_SPI_handler(nRF5x_SPI2, p_event);
}
// Обработчик приема данных экземпляра SPI3
void nRF5x_SPI_3_handler(nrf_drv_spi_evt_t const * p_event, void* p_context) {
  nRF5x_SPI_handler(nRF5x_SPI3, p_event);
}

//-----------------------------------------------------------------------------
// Ожидание передачи байта
uint8_t nRF5x_SPI_wait_xfer(uint8_t instance, uint32_t timeout_us) {
  while(spi_xfer_done[instance]) {
    nrf_delay_us(1);
    if(!timeout_us) return false;
    timeout_us--;
  }
  return true;
}

//-----------------------------------------------------------------------------
// Инициализация порта SPI
uint8_t nRF5x_SPI_begin(uint8_t instance, uint8_t mosi, uint8_t miso, uint8_t sck) {
  // Экземпляр UART не может быть больше чем имеется в МК
  if(instance >= SPI_INSTANCE_COUNT) return false;
  
  // конфигурация пинов
  nRF5x_spi_config[instance].mosi_pin = mosi;
  nRF5x_spi_config[instance].miso_pin = miso;
  nRF5x_spi_config[instance].sck_pin  = sck;
  // дефолтные значения ----------------------------------------
  nRF5x_spi_config[instance].ss_pin       = NRF_DRV_SPI_PIN_NOT_USED;
  nRF5x_spi_config[instance].orc          = 0xFF;
  nRF5x_spi_config[instance].def          = SPIS_DEFAULT_DEF;
  nRF5x_spi_config[instance].mode         = NRF_DRV_SPI_MODE_0;
  nRF5x_spi_config[instance].miso_drive   = NRF_DRV_SPIS_DEFAULT_MISO_DRIVE;
  nRF5x_spi_config[instance].frequency    = NRF_DRV_SPI_FREQ_4M;
  nRF5x_spi_config[instance].bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST;
  nRF5x_spi_config[instance].irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY;
  
  return nRF5x_SPI_init(instance, &nRF5x_spi_config[instance]);
  
}

//-----------------------------------------------------------------------------
// Инициализация порта SPI через структуры конфигурации
uint8_t nRF5x_SPI_init(uint8_t instance, nRF5x_spi_config_t* config) {
  
  ret_code_t status = NRF_SUCCESS;
  
  // Экземпляр UART не может быть больше чем имеется в МК
  if(instance >= SPI_INSTANCE_COUNT) return false;
  
  // Получение структуры экземпляра UART по номеру
  nrf_drv_spi_t spi = *nrf_drv_spi_get_instance(instance);
  
  // Переопределяем стуктуру конфигурации для экземпляра
  nRF5x_spi_config[instance] = *config;
    
  spi_xfer_done[instance] = false;
  
  nrf_drv_spi_evt_handler_t handler;
  switch(instance) {
    case nRF5x_SPI0 : handler = nRF5x_SPI_0_handler; break;
    case nRF5x_SPI1 : handler = nRF5x_SPI_1_handler; break;
    case nRF5x_SPI2 : handler = nRF5x_SPI_2_handler; break;
    default         : handler = nRF5x_SPI_3_handler; break;
  }
  
  status = nrf_drv_spi_init(&spi, &nRF5x_spi_config[instance], handler, NULL);
  if(status != NRF_SUCCESS) return false;
  
  nRF5x_SPI_inited_flag[instance] = 1;
  
  return (status==NRF_SUCCESS);
}

//-----------------------------------------------------------------------------
// Деинициализация порта SPI
uint8_t nRF5x_SPI_deinit(uint8_t instance) {
  if(instance >= SPI_INSTANCE_COUNT) return false;
  if(nRF5x_SPI_inited_flag[instance]) {
    nrf_drv_spi_t spi = *nrf_drv_spi_get_instance(instance);
    nrf_drv_spi_uninit(&spi);
    nRF5x_SPI_inited_flag[instance] = 0;
  }
  return !nRF5x_SPI_inited_flag[instance];
}

//-----------------------------------------------------------------------------
// Конфигурация режима работы
uint8_t nRF5x_SPI_setMode(uint8_t instance, uint8_t mode) {
  if(mode > NRF_SPI_MODE_3) return false;
  if(!nRF5x_SPI_deinit(instance)) return false;
  nRF5x_spi_config[instance].mode = (nrf_drv_spi_mode_t)mode;
  return nRF5x_SPI_init(instance, &nRF5x_spi_config[instance]);
}

//-----------------------------------------------------------------------------
// Конфигурация последовательности бит
uint8_t nRF5x_SPI_setBitOrder(uint8_t instance, uint8_t bitOrder) {
  if(bitOrder > NRF_DRV_SPI_BIT_ORDER_LSB_FIRST) return false;
  if(!nRF5x_SPI_deinit(instance)) return false;
  nRF5x_spi_config[instance].bit_order = (nrf_drv_spi_bit_order_t)bitOrder;
  return nRF5x_SPI_init(instance, &nRF5x_spi_config[instance]);
}

//-----------------------------------------------------------------------------
// Конфигурация частоты
uint8_t nRF5x_SPI_setFreq(uint8_t instance, nrf_drv_spi_frequency_t freq) {
  if(freq > NRF_DRV_SPI_FREQ_8M) return false;
  if(!nRF5x_SPI_deinit(instance)) return false;
  nRF5x_spi_config[instance].frequency = freq;
  return nRF5x_SPI_init(instance, &nRF5x_spi_config[instance]);
}

//-----------------------------------------------------------------------------
// Конфигурация аппаратного пина NSS
uint8_t nRF5x_SPI_setNSS(uint8_t instance, uint8_t pinNSS) {
  if(!nRF5x_SPI_deinit(instance)) return false;
  nRF5x_spi_config[instance].ss_pin = pinNSS;
  return nRF5x_SPI_init(instance, &nRF5x_spi_config[instance]);
}

//-----------------------------------------------------------------------------
// Передача данных по SPI
uint8_t nRF5x_SPI_writeRead(uint8_t instance,
                            uint8_t* dataTX, uint8_t  sizeTX,
                            uint8_t* dataRX, uint8_t  sizeRX)
{
  if(instance >= SPI_INSTANCE_COUNT) return false;
  if(!nRF5x_SPI_inited_flag[instance]) return false;
  nrf_drv_spi_t spi = *nrf_drv_spi_get_instance(instance);
  spi_xfer_done[instance] = false;
  ret_code_t status = nrf_drv_spi_transfer(&spi, dataTX, sizeTX, dataRX, sizeRX);
  return nRF5x_SPI_wait_xfer(instance, 1000);
}

//-----------------------------------------------------------------------------
// Передача байта по SPI, возвращает ответный байт
uint8_t nRF5x_SPI_transfer(uint8_t instance, uint8_t byte) {
  uint8_t byteRX[1];
  uint8_t byteTX[1];
  byteTX[0] = byte;
  if(!nRF5x_SPI_writeRead(instance, &byteTX[0], 1, &byteRX[0], 1)) return 0;
  return byteRX[0];
}
