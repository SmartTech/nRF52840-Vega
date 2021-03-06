#ifndef _NRF5x_USB_GENERIC_H_
#define _NRF5x_USB_GENERIC_H_

#include "nRF5x_USB.h"

#include "app_usbd_hid_generic.h"

#ifdef __cplusplus
extern "C" {
#endif


uint8_t nRF5x_USB_GENERIC_init(void);
void    nRF5x_USB_GENERIC_handle(void);


#ifdef __cplusplus
}
#endif

#endif // _NRF5x_USB_GENERIC_H_
