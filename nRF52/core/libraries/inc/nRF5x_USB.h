#ifndef _NRF5x_USB_H_
#define _NRF5x_USB_H_
#ifdef NRF52840_XXAA  // USB есть только у серии nRF52840

uint8_t nRF5x_USB_CDC_init(void);
uint8_t nRF5x_USB_MSC_init(void);
uint8_t nRF5x_USB_AUDIO_init(void);
uint8_t nRF5x_USB_MOUSE_init(void);
uint8_t nRF5x_USB_KEABOARD_init(void);

#endif // NRF52840_XXAA
#endif // _NRF5x_USB_H_