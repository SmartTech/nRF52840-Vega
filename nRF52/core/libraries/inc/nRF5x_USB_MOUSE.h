#ifndef _NRF5x_USB_MOUSE_H_
#define _NRF5x_USB_MOUSE_H_

#include "nRF5x_USB.h"

#include "app_usbd_hid_mouse.h"


#define CONFIG_MOUSE_BUTTON_COUNT 2


#ifdef __cplusplus
extern "C" {
#endif


uint8_t nRF5x_USB_MOUSE_init(void);



#ifdef __cplusplus
}
#endif

#endif // _NRF5x_USB_MOUSE_H_
