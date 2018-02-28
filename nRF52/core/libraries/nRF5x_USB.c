#include "nRF5x_USB.h"

#define SERIAL_NUMBER_STRING_SIZE (16)

char nRF5x_USB_m_rx_buffer[NRF_DRV_USBD_EPSIZE * 16];
char nRF5x_USB_m_tx_buffer[NRF_DRV_USBD_EPSIZE * 16];

uint16_t nRF5x_USB_g_extern_serial_number[SERIAL_NUMBER_STRING_SIZE + 1];

bool nRF5x_USB_connected = false;

//----------------------------------------------------------------------------
void nRF5x_USB_power_event_handler(nrf_drv_power_usb_evt_t event) {
    switch (event) {
        case NRF_DRV_POWER_USB_EVT_DETECTED:
            //NRF_LOG_INFO("USB power detected");
            if (!nrf_drv_usbd_is_enabled()) {
                app_usbd_enable();
            }
            break;
        case NRF_DRV_POWER_USB_EVT_REMOVED:
            //NRF_LOG_INFO("USB power removed");
            nRF5x_USB_connected = false;
            break;
        case NRF_DRV_POWER_USB_EVT_READY:
            //NRF_LOG_INFO("USB ready");
            nRF5x_USB_connected = true;
            break;
        default: ASSERT(false);
    }
}

//-----------------------------------------------------------------------------
void nRF5x_USB_ev_handler(app_usbd_event_type_t event) {
    switch (event) {
        case APP_USBD_EVT_DRV_SOF:     break;
        case APP_USBD_EVT_DRV_SUSPEND: break;
        case APP_USBD_EVT_DRV_RESUME:  break;
        case APP_USBD_EVT_STARTED:     break;
        case APP_USBD_EVT_STOPPED:     app_usbd_disable(); break;
        default: break;
    }
}

//-----------------------------------------------------------------------------
void nRF5x_USB_start(void) {
    if (USBD_POWER_DETECTION) {
        static const nrf_drv_power_usbevt_config_t config = {
            .handler =  nRF5x_USB_power_event_handler
        };
        ret_code_t ret;
        ret = nrf_drv_power_usbevt_init(&config);
        APP_ERROR_CHECK(ret);
    }
    else {
        //NRF_LOG_INFO("No USB power detection enabled\r\nStarting USB now");
        app_usbd_enable();
        app_usbd_start();
        nRF5x_USB_connected = true;
    }
}

//-----------------------------------------------------------------------------
void nRF5x_USB_serial_number_string_create(void)
{
    static uint8_t nRF5x_USB_serial_number_created = false;
    if(nRF5x_USB_serial_number_created) return;
    nRF5x_USB_g_extern_serial_number[0] = (uint16_t)APP_USBD_DESCRIPTOR_STRING << 8 |
                                          sizeof(nRF5x_USB_g_extern_serial_number);

    uint32_t dev_id_hi = NRF_FICR->DEVICEID[1];
    uint32_t dev_id_lo = NRF_FICR->DEVICEID[0];

    uint64_t device_id = (((uint64_t)dev_id_hi) << 32) | dev_id_lo;

    for (size_t i = 1; i < SERIAL_NUMBER_STRING_SIZE + 1; ++i)
    {
        char tmp[2];
        (void)snprintf(tmp, sizeof(tmp), "%x", (unsigned int)(device_id & 0xF));
        nRF5x_USB_g_extern_serial_number[i] = tmp[0];
        device_id >>= 4;
    }
    nRF5x_USB_serial_number_created = true;
}

//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_init(void) {
  
  static uint8_t nRF5x_USB_inited = false;
  
  static const app_usbd_config_t usbd_config = {
  .ev_state_proc =  nRF5x_USB_ev_handler, };
  
  if(nRF5x_USB_inited) return false;
  
  nRF5x_USB_serial_number_string_create();
  
  app_usbd_init(&usbd_config);
  
  return nRF5x_USB_inited = true;
}

//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_CDC_init(void) {
  
  nRF5x_USB_init();
  
  return false;
}

uint8_t nRF5x_USB_MSC_init(void) {
  
  nRF5x_USB_init();
  
  return false;
}

uint8_t nRF5x_USB_AUDIO_init(void) {
  
  nRF5x_USB_init();
  
  return false;
}

uint8_t nRF5x_USB_MOUSE_init(void) {
  
  nRF5x_USB_init();
  
  return false;
}

uint8_t nRF5x_USB_KEABOARD_init(void) {
  
  nRF5x_USB_init();
  
  return false;
}
