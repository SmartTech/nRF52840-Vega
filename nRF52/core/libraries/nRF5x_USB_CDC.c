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

static char m_rx_buffer[NRF_DRV_USBD_EPSIZE * 16];

APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            nRF5x_CDC_ACM_INTERFACES_CONFIG(),
                            nRF5x_USB_cdc_ev_handler,
                            m_cdc_acm_class_descriptors
);



//-----------------------------------------------------------------------------
void nRF5x_USB_cdc_ev_handler(app_usbd_class_inst_t const * p_inst,
                              app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            /*Setup first transfer*/
            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   m_rx_buffer,
                                                   sizeof(m_rx_buffer));
            APP_ERROR_CHECK(ret);
            break;
        }
        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            break;
        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            break;
        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            /*Get amount of data transfered*/
            size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);


            /*Setup next transfer*/
            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   m_rx_buffer,
                                                   sizeof(m_rx_buffer));

            ASSERT(ret == NRF_SUCCESS); /*Should not happen*/
            break;
        }
        default:
            break;
    }
}


//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_CDC_init(void) {
  
  nRF5x_USB_init();
  
  if(nRF5x_USB_connected()) nRF5x_USB_stop();
  
  app_usbd_class_inst_t const * class_cdc_acm;
  class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
  app_usbd_class_append(class_cdc_acm);

  nRF5x_USB_start();
  
  return true;
}
