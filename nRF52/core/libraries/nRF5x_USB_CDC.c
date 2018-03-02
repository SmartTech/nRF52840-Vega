#include "nRF5x_USB_CDC.h"

#define nRF5x_USB_CDC_COMM_EPIN         NRF_DRV_USBD_EPIN4
#define nRF5x_USB_CDC_DATA_EPIN         NRF_DRV_USBD_EPIN3
#define nRF5x_USB_CDC_DATA_EPOUT        NRF_DRV_USBD_EPOUT3


#define nRF5x_CDC_ACM_INTERFACES_CONFIG()                 \
    APP_USBD_CDC_ACM_CONFIG(nRF5x_USB_INTERFACE_CDC_COMM, \
                            nRF5x_USB_CDC_COMM_EPIN,      \
                            nRF5x_USB_INTERFACE_CDC_DATA, \
                            nRF5x_USB_CDC_DATA_EPIN,      \
                            nRF5x_USB_CDC_DATA_EPOUT)
    
static const uint8_t m_cdc_acm_class_descriptors[] = {
        APP_USBD_CDC_ACM_DEFAULT_DESC(nRF5x_USB_INTERFACE_CDC_COMM,
                                      nRF5x_USB_CDC_COMM_EPIN,
                                      nRF5x_USB_INTERFACE_CDC_DATA,
                                      nRF5x_USB_CDC_DATA_EPIN,
                                      nRF5x_USB_CDC_DATA_EPOUT)
};

static void nRF5x_USB_cdc_ev_handler(app_usbd_class_inst_t const * p_inst,
                                     app_usbd_cdc_acm_user_event_t event);

uint8_t nRF5x_USB_cdc_rx_buffer[NRF_DRV_USBD_EPSIZE * 16];

APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            nRF5x_CDC_ACM_INTERFACES_CONFIG(),
                            nRF5x_USB_cdc_ev_handler,
                            m_cdc_acm_class_descriptors
);

nRF5x_USB_CDC_callback_t nRF5x_USB_CDC_event;    // вызвается из события 
nRF5x_USB_CDC_callback_t nRF5x_USB_CDC_callback; // вызывается из обработчика

uint8_t nRF5x_USB_CDC_opened  = false;
uint8_t nRF5x_USB_CDC_inited  = false;
uint8_t nRF5x_USB_CDC_tx_flag = false;

//-----------------------------------------------------------------------------
void nRF5x_USB_cdc_ev_handler(app_usbd_class_inst_t const * p_inst,
                              app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            nRF5x_USB_CDC_opened = true;
            /*Setup first transfer*/
            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   nRF5x_USB_cdc_rx_buffer,
                                                   sizeof(nRF5x_USB_cdc_rx_buffer));
            APP_ERROR_CHECK(ret);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            nRF5x_USB_CDC_opened = false;
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            nRF5x_USB_CDC_tx_flag = false;
            break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            /*Get amount of data transfered*/
            size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
            
            if(nRF5x_USB_CDC_event.attached) {
              nRF5x_USB_CDC_event.rx_callback(nRF5x_USB_cdc_rx_buffer, size);
            }
            
            
            // Если подключен callback, то выставляем флаг, запуск приема
            // после выполнения callback функции
            if(nRF5x_USB_CDC_callback.attached) {
              nRF5x_USB_CDC_callback.flag = true;
              nRF5x_USB_CDC_callback.size = size;
            }
            // иначае сразу запускаем следующий прием
            else {
              app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                    nRF5x_USB_cdc_rx_buffer,
                                    sizeof(nRF5x_USB_cdc_rx_buffer));
            }

            //ASSERT(ret == NRF_SUCCESS); /*Should not happen*/
            break;
        }
        default: break;
    }
}


//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_CDC_init(void) {
  
  if(nRF5x_USB_CDC_inited) return false;
  
  nRF5x_USB_CDC_event.attached    = false;
  nRF5x_USB_CDC_callback.attached = false;
  nRF5x_USB_CDC_callback.flag     = false;
  
  nRF5x_USB_init();
  
  if(nRF5x_USB_connected()) nRF5x_USB_stop();
  
  app_usbd_class_inst_t const * class_cdc_acm;
  class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
  app_usbd_class_append(class_cdc_acm);

  nRF5x_USB_start();
  
  return nRF5x_USB_CDC_inited = true;
}

uint8_t nRF5x_USB_CDC_deinit(void) {
  if(!nRF5x_USB_CDC_inited) return false;
  return true;
}

void nRF5x_USB_CDC_attachEvent(nRF5x_USB_CDC_eventCallback function)
{
  nRF5x_USB_CDC_event.rx_callback = function;
  nRF5x_USB_CDC_event.attached = true;
}

void nRF5x_USB_CDC_attachCallback(nRF5x_USB_CDC_eventCallback function)
{
  nRF5x_USB_CDC_callback.rx_callback = function;
  nRF5x_USB_CDC_callback.attached = true;
}

size_t nRF5x_USB_CDC_available(void)
{
  return false;
}

uint8_t nRF5x_USB_CDC_read(uint8_t* byte, size_t size)
{
  return false;
}

uint8_t nRF5x_USB_CDC_write(uint8_t* data, size_t size)
{
  if(!nRF5x_USB_CDC_opened) return false;
  if(nRF5x_USB_CDC_callback.attached) {
    uint16_t timeout_us = 1000;
    while(nRF5x_USB_CDC_tx_flag) {
      nrf_delay_us(1);
      if(!timeout_us) return false;
      timeout_us--;
    }
  }
  uint8_t status = app_usbd_cdc_acm_write(&m_app_cdc_acm, data, size);
  nRF5x_USB_CDC_tx_flag = true;
  return (status == NRF_SUCCESS);
}

uint8_t nRF5x_USB_CDC_print(const char* data)
{
  if(!nRF5x_USB_CDC_opened) return false;
  return nRF5x_USB_CDC_write((uint8_t*)data, strlen(data));
}

void nRF5x_USB_CDC_handle(void) {
  if(!nRF5x_USB_CDC_callback.attached) return;
  if(!nRF5x_USB_CDC_callback.flag) return;
  nRF5x_USB_CDC_callback.flag = false;
  nRF5x_USB_CDC_callback.rx_callback(nRF5x_USB_cdc_rx_buffer, nRF5x_USB_CDC_callback.size);
  app_usbd_cdc_acm_read(&m_app_cdc_acm,
                        nRF5x_USB_cdc_rx_buffer,
                        sizeof(nRF5x_USB_cdc_rx_buffer));
}
