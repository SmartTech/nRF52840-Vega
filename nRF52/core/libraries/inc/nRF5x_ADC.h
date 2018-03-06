#ifndef _NRF5x_ADC_H_
#define _NRF5x_ADC_H_

#include "nrf_drv_saadc.h"


#define nRF5x_ADC_8BIT             NRF_SAADC_RESOLUTION_8BIT 
#define nRF5x_ADC_10BIT            NRF_SAADC_RESOLUTION_10BIT
#define nRF5x_ADC_12BIT            NRF_SAADC_RESOLUTION_12BIT
#define nRF5x_ADC_14BIT            NRF_SAADC_RESOLUTION_14BIT

#define nRF5x_ADC_config_t         nrf_drv_saadc_config_t
#define nRF5x_ADC_channel_t        nrf_saadc_channel_config_t

#define nRF5x_ADC_CHANNEL_COUNT    NRF_SAADC_CHANNEL_COUNT

#ifdef __cplusplus
extern "C" {
#endif

  
void     nRF5x_ADC_init(nrf_drv_saadc_config_t adcConfig);
void     nRF5x_ADC_deinit();

void     nRF5x_ADC_attachCallback(uint8_t channel);

uint16_t nRF5x_ADC_read(uint8_t channel);


#ifdef __cplusplus
}
#endif

#endif // _NRF5x_ADC_H_