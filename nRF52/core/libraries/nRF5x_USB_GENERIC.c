#include "nRF5x_USB_GENERIC.h"


#define nRF5x_USB_HID_GENERIC_EPIN      NRF_DRV_USBD_EPIN1

#define nRF5x_HID_GENERIC_EP_COUNT      1

#define nRF5x_REPORT_IN_QUEUE_SIZE      1
#define nRF5x_REPORT_OUT_MAXSIZE        0


//-----------------------------------------------------------------------------
#if nRF5x_USB_USE_GENERIC
static const uint8_t m_hid_generic_rep_descriptor[] = 
        APP_USBD_HID_MOUSE_REPORT_DSC_BUTTON(2);

static const uint8_t m_hid_generic_class_descriptors[] = {
        APP_USBD_HID_GENERIC_INTERFACE_DSC(nRF5x_USB_INTERFACE_GENERIC,
                                           nRF5x_HID_GENERIC_EP_COUNT,
                                           APP_USBD_HID_SUBCLASS_BOOT,
                                           APP_USBD_HID_PROTO_MOUSE)
        APP_USBD_HID_GENERIC_HID_DSC(m_hid_generic_rep_descriptor)
        APP_USBD_HID_GENERIC_EP_DSC(nRF5x_USB_HID_GENERIC_EPIN)
};

static void nRF5x_USB_generic_ev_handler(app_usbd_class_inst_t const * p_inst,
                                         app_usbd_hid_user_event_t event);

APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_generic,
                                nRF5x_USB_INTERFACE_GENERIC,
                                nRF5x_USB_generic_ev_handler,
                                nRF5x_USB_HID_GENERIC_EPIN,
                                m_hid_generic_class_descriptors,
                                m_hid_generic_rep_descriptor,
                                nRF5x_REPORT_IN_QUEUE_SIZE,
                                nRF5x_REPORT_OUT_MAXSIZE);
);

void nRF5x_USB_generic_ev_handler(app_usbd_class_inst_t const * p_inst,
                                  app_usbd_hid_user_event_t event)
{
  
}
#endif

//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_GENERIC_init(void) {
  
#if nRF5x_USB_USE_GENERIC
  nRF5x_USB_init();
  
  if(nRF5x_USB_connected) nRF5x_USB_stop();
  
  app_usbd_class_inst_t const * class_inst_generic;
  class_inst_generic = app_usbd_hid_generic_class_inst_get(&m_app_hid_generic);
  app_usbd_class_append(class_inst_generic);
  
  nRF5x_USB_start();
#endif
  
  return true;
}
