#ifndef _NRF5x_USB_H_
#define _NRF5x_USB_H_

#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_string_desc.h"

#include "nrf_drv_power.h"

#include "nRF5x_USB_CDC.h"
#include "nRF5x_USB_MSC.h"
#include "nRF5x_USB_AUDIO.h"
#include "nRF5x_USB_MOUSE.h"
#include "nRF5x_USB_KEYBOARD.h"
#include "nRF5x_USB_GENERIC.h"

#ifdef __cplusplus
extern "C" {
#endif

#define nRF5x_USB_USE_CDC               1
#define nRF5x_USB_USE_MSC               1
#define nRF5x_USB_USE_AUDIO             0
#define nRF5x_USB_USE_MOUSE             0
#define nRF5x_USB_USE_KEABOARD          0
#define nRF5x_USB_USE_GENERIC           1


#define nRF5x_USB_INTERFACE_MOUSE       0
#define nRF5x_USB_INTERFACE_KBD         1
#define nRF5x_USB_INTERFACE_GENERIC     2
#define nRF5x_USB_INTERFACE_CDC_COMM    3
#define nRF5x_USB_INTERFACE_CDC_DATA    4

#define USBD_POWER_DETECTION true


uint8_t nRF5x_USB_init(void);

uint8_t nRF5x_USB_connected(void);

void    nRF5x_USB_stop(void);
void    nRF5x_USB_start(void);
void    nRF5x_USB_handle(void);


#ifdef __cplusplus
}
#endif

#endif // _NRF5x_USB_H_
