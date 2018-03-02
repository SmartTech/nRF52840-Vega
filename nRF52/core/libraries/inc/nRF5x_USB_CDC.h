#ifndef _NRF5x_USB_CDC_H_
#define _NRF5x_USB_CDC_H_

#include "nRF5x_USB.h"

#include "app_usbd_cdc_acm.h"
#include "nrf_delay.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nRF5x_USB_CDC_eventCallback)(uint8_t* dataPtr, size_t size);

typedef struct {
  nRF5x_USB_CDC_eventCallback rx_callback;
  uint8_t                     attached;
  uint8_t                     flag;
  uint8_t                     size;
} nRF5x_USB_CDC_callback_t;


uint8_t nRF5x_USB_CDC_init(void);
uint8_t nRF5x_USB_CDC_deinit(void);

void    nRF5x_USB_CDC_attachEvent   (nRF5x_USB_CDC_eventCallback function);
void    nRF5x_USB_CDC_attachCallback(nRF5x_USB_CDC_eventCallback function);

uint8_t nRF5x_USB_CDC_write(uint8_t* data, size_t size);
uint8_t nRF5x_USB_CDC_print(const char* data);

uint8_t nRF5x_USB_CDC_get(uint8_t* byte);
uint8_t nRF5x_USB_CDC_read(uint8_t* byte, size_t size);

size_t  nRF5x_USB_CDC_available();

void    nRF5x_USB_CDC_handle();

#ifdef __cplusplus
}
#endif

#endif // _NRF5x_USB_CDC_H_
