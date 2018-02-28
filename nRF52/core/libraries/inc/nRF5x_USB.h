#ifndef _NRF5x_USB_H_
#define _NRF5x_USB_H_
#ifdef NRF52840_XXAA  // USB есть только у серии nRF52840

#include "nrf.h"

#include "nrf_drv_power.h"
#include "nrf_drv_usbd.h"

#include "app_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd_hid_generic.h"
#include "app_usbd_hid_mouse.h"
#include "app_usbd_hid_kbd.h"

#ifndef USBD_POWER_DETECTION
#define USBD_POWER_DETECTION true
#endif

#ifdef _cplusplus
extern "C" {
#endif

uint8_t nRF5x_USB_CDC_init(void);
uint8_t nRF5x_USB_MSC_init(void);
uint8_t nRF5x_USB_AUDIO_init(void);
uint8_t nRF5x_USB_MOUSE_init(void);
uint8_t nRF5x_USB_KEABOARD_init(void);

#ifdef _cplusplus
}
#endif

#endif // NRF52840_XXAA
#endif // _NRF5x_USB_H_
