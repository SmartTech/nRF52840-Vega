#include "nRF5x_USB.h"
                                       

#define SERIAL_NUMBER_STRING_SIZE       16


uint16_t g_extern_serial_number[SERIAL_NUMBER_STRING_SIZE + 1];

uint8_t nRF5x_USB_isConnected = false;


//-----------------------------------------------------------------------------
void nRF5x_USB_ev_handler(app_usbd_event_type_t event) {
    switch (event) {
        case APP_USBD_EVT_DRV_SOF:
          if(APP_USBD_STATE_Configured != app_usbd_core_state_get()) break;
          /*size_t rx_size = app_usbd_audio_class_rx_size_get(&m_app_audio_headphone.base);
          m_temp_buffer_size = rx_size;
          if (rx_size > 0) {
              ASSERT(rx_size <= sizeof(m_temp_buffer));
              ret_code_t ret;
              ret = app_usbd_audio_class_rx_start(&m_app_audio_headphone.base, m_temp_buffer, rx_size);
              if (NRF_SUCCESS != ret) {
                  // Cannot start RX transfer from headphone
              }
          }*/
          break;
        case APP_USBD_EVT_DRV_SUSPEND: app_usbd_suspend_req(); break;
        case APP_USBD_EVT_DRV_RESUME:  break;
        case APP_USBD_EVT_STARTED:     break;
        case APP_USBD_EVT_STOPPED:     app_usbd_disable();  break;
        default: break;
    }
}

//-----------------------------------------------------------------------------
void nRF5x_USB_power_event_handler(nrf_drv_power_usb_evt_t event) {
  switch (event) {
    case NRF_DRV_POWER_USB_EVT_DETECTED:
        if (!nrf_drv_usbd_is_enabled()) app_usbd_enable();
        break;
    case NRF_DRV_POWER_USB_EVT_REMOVED:
        nRF5x_USB_isConnected = false;
        break;
    case NRF_DRV_POWER_USB_EVT_READY:
        nRF5x_USB_isConnected = true;
        break;
    default: ASSERT(false);
  }
}

//=============================================================================
void nRF5x_USB_start(void) {
  if (USBD_POWER_DETECTION) {
      static const nrf_drv_power_usbevt_config_t config = {
          .handler = nRF5x_USB_power_event_handler
      };
      ret_code_t ret;
      ret = nrf_drv_power_usbevt_init(&config);
      APP_ERROR_CHECK(ret);
  }
  else {
      app_usbd_enable();
      app_usbd_start();
      nRF5x_USB_isConnected = true;
  }
}

//-----------------------------------------------------------------------------
void nRF5x_USB_stop(void) {
  app_usbd_disable();
  app_usbd_stop();
  nRF5x_USB_isConnected = false;
}

uint8_t nRF5x_USB_connected(void) {
  return nRF5x_USB_isConnected;
}

//-----------------------------------------------------------------------------
void nRF5x_USB_connection_handle() {
  static uint8_t nRF5x_last_conn_status = false;
  if (nRF5x_last_conn_status != nRF5x_USB_isConnected) {
      nRF5x_last_conn_status = nRF5x_USB_isConnected;
      nRF5x_USB_isConnected ? app_usbd_start() : app_usbd_stop();
  }
}

//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_init(void) {
  static uint8_t nRF5x_USB_inited = false;
  if(nRF5x_USB_inited) return false;
  static const app_usbd_config_t usbd_config = {
        .ev_state_proc = nRF5x_USB_ev_handler,
        .enable_sof = true
    };
  //serial_number_string_create();
  app_usbd_init(&usbd_config);
  return nRF5x_USB_inited = true;
}

//-----------------------------------------------------------------------------
void nRF5x_USB_handle(void) {
    while (app_usbd_event_queue_process()) { /* Nothing to do */ }
    nRF5x_USB_connection_handle();
}
