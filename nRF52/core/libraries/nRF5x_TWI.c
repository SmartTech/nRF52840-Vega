#include "nRF5x_TWI.h"

//-----------------------------------------------------------------------------
// Флгаи инициализации экземпляров UART
uint8_t nRF5x_TWI_inited_flag[TWI_INSTANCE_COUNT] = {false, false};

//-----------------------------------------------------------------------------
nRF5x_twi_config_t nRF5x_twi_config[TWI_INSTANCE_COUNT] = {
                                                    nRF5x_TWI_DEFAULT_CONFIG,
                                                    nRF5x_TWI_DEFAULT_CONFIG};

//-----------------------------------------------------------------------------
// Инициализация порта TWI
uint8_t nRF5x_TWI_begin(uint8_t instance, uint8_t _pinSDA, uint8_t _pinSCL)
{  
  // Экземпляр TWI не может быть больше чем имеется в МК
  if(instance >= TWI_INSTANCE_COUNT) return false;
  
  // конфигурация пинов
  nRF5x_twi_config[instance].sda = _pinSDA;
  nRF5x_twi_config[instance].scl = _pinSCL;
  
  return nRF5x_TWI_init(instance, &nRF5x_twi_config[instance]);
}

//-----------------------------------------------------------------------------
// Инициализация порта TWI через структуру конфигурации
uint8_t nRF5x_TWI_init(uint8_t instance, nRF5x_twi_config_t* config) {
  
  uint8_t status = NRF_SUCCESS;
  
  // Экземпляр TWI не может быть больше чем имеется в МК
  if(instance >= TWI_INSTANCE_COUNT) return false;
  
  // Получение структуры экземпляра UART по номеру
  nrf_drv_twi_t twi = *nrf_drv_twi_get_instance(instance);
  
  // Переопределяем стуктуру конфигурации для экземпляра
  nRF5x_twi_config[instance] = *config;
  
  status = nrf_drv_twi_init(&twi, &nRF5x_twi_config[instance], NULL, NULL);
  if(status != NRF_SUCCESS) return false;
  
  nRF5x_TWI_inited_flag[instance] = 1;
  
  nrf_drv_twi_enable(&twi);
  
  return (status==NRF_SUCCESS);
}

//-----------------------------------------------------------------------------
// Деинициализация порта TWI
uint8_t nRF5x_TWI_deinit(uint8_t instance)
{
  if(instance >= TWI_INSTANCE_COUNT) return false;
  if(nRF5x_TWI_inited_flag[instance]) {
    nrf_drv_twi_t twi = *nrf_drv_twi_get_instance(instance);
    nrf_drv_twi_uninit(&twi);
    nRF5x_TWI_inited_flag[instance] = 0;
  }
  return !nRF5x_TWI_inited_flag[instance];
}

//-----------------------------------------------------------------------------
// Конфигурация частоты TWI
uint8_t nRF5x_TWI_setFreq (uint8_t instance, nrf_twi_frequency_t freq) {
  switch(freq) {
    case NRF_TWI_FREQ_100K : break;
    case NRF_TWI_FREQ_250K : break;
    case NRF_TWI_FREQ_400K : break;
    default: return false;
  }
  if(!nRF5x_TWI_deinit(instance)) return false;
  nRF5x_twi_config[instance].frequency = freq;
  return nRF5x_TWI_init(instance, &nRF5x_twi_config[instance]);
}

//-----------------------------------------------------------------------------
// Конфигурация риоритета прерываний TWI
uint8_t nRF5x_TWI_setPriority(uint8_t instance, uint8_t priority) {
  if(priority > 7) priority = 7;
  if(!nRF5x_TWI_deinit(instance)) return false;
  nRF5x_twi_config[instance].interrupt_priority = priority;
  return nRF5x_TWI_init(instance, &nRF5x_twi_config[instance]);
}

uint8_t nRF5x_TWI_beginTransmission(uint8_t instance, uint8_t address) {
  if(instance >= TWI_INSTANCE_COUNT) return false;
  nrf_drv_twi_t twi = *nrf_drv_twi_get_instance(instance);
  uint8_t data;
  return (nrf_drv_twi_rx(&twi, address, &data, sizeof(data)) == NRF_SUCCESS);
}

uint8_t nRF5x_TWI_endTransmission(uint8_t instance) {
  return true;
}

uint8_t nRF5x_TWI_write(uint8_t instance, uint8_t* data, uint8_t size) {
  return false;
}

uint8_t nRF5x_TWI_read (uint8_t instance, uint8_t* data, uint8_t size) {
  return false;
}

uint8_t nRF5x_TWI_available(uint8_t instance) {
  return false;
}

uint8_t nRF5x_TWI_requastFrom(uint8_t instance, uint8_t address) {
  return false;
}
