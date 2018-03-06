#include "nRF5x_ADC.h"

uint8_t nRF5x_ADC_inited = false;
uint8_t nRF5x_ADC_chInited[8];

nRF5x_ADC_config_t adcConfig = NRF_DRV_SAADC_DEFAULT_CONFIG;

/*
#define NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(PIN_P) \
    {                                                  \
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,     \
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,     \
        .gain       = NRF_SAADC_GAIN1_6,               \
        .reference  = NRF_SAADC_REFERENCE_INTERNAL,    \
        .acq_time   = NRF_SAADC_ACQTIME_10US,          \
        .mode       = NRF_SAADC_MODE_SINGLE_ENDED,     \
        .burst      = NRF_SAADC_BURST_DISABLED,        \
        .pin_p      = (nrf_saadc_input_t)(PIN_P),      \
        .pin_n      = NRF_SAADC_INPUT_DISABLED         \
    }

#define NRF_DRV_SAADC_DEFAULT_CONFIG                                        \
    {                                                                       \
    .resolution         = (nrf_saadc_resolution_t)SAADC_CONFIG_RESOLUTION,  \
    .oversample         = (nrf_saadc_oversample_t)SAADC_CONFIG_OVERSAMPLE,  \
    .interrupt_priority = SAADC_CONFIG_IRQ_PRIORITY,                        \
    .low_power_mode     = SAADC_CONFIG_LP_MODE                              \
    }

*/

void nRF5x_ADC_eventHandler(nrf_drv_saadc_evt_t const * p_event) {
  
}

void nRF5x_ADC_init(nRF5x_ADC_config_t _config) {
  
  if(nRF5x_ADC_inited) nRF5x_ADC_deinit();
  
  adcConfig = _config;
  
  nrf_drv_saadc_init(&adcConfig, nRF5x_ADC_eventHandler);
    
}

void nRF5x_ADC_deinit() {
  
  nrf_drv_saadc_uninit();
  
  adcConfig.resolution         = SAADC_CONFIG_RESOLUTION;
  adcConfig.oversample         = SAADC_CONFIG_OVERSAMPLE;
  adcConfig.interrupt_priority = SAADC_CONFIG_IRQ_PRIORITY;
  adcConfig.low_power_mode     = SAADC_CONFIG_LP_MODE;
  
}

void nRF5x_ADC_begin(uint8_t channel) {
  
  if(channel >= nRF5x_ADC_CHANNEL_COUNT) return;
  
  if(!nRF5x_ADC_inited) return;
  
  nRF5x_ADC_channel_t config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(channel + 1);
  
  nrf_drv_saadc_channel_init(channel, &config);
  nRF5x_ADC_chInited[channel] = true;
  
}

uint16_t nRF5x_ADC_read(uint8_t channel) {
  
  if(channel >= nRF5x_ADC_CHANNEL_COUNT) return 0;
  
  if(!nRF5x_ADC_chInited[channel]) return 0;
  
  return true;
  
}
