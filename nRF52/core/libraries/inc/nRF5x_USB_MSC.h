#ifndef _NRF5x_USB_MSC_H_
#define _NRF5x_USB_MSC_H_

#include "nRF5x_USB.h"

#include "app_usbd_msc.h"
#include "nrf_block_dev_ram.h"

#ifdef __cplusplus
extern "C" {
#endif


uint8_t nRF5x_USB_MSC_init(void);


#ifdef __cplusplus
}
#endif

#endif // _NRF5x_USB_MSC_H_
