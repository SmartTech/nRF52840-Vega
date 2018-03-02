#include "nRF5x_USB_MOUSE.h"

static void nRF5x_USB_mouse_ev_handler(app_usbd_class_inst_t const * p_inst,
                                       app_usbd_hid_user_event_t event);

APP_USBD_HID_MOUSE_GLOBAL_DEF(m_app_hid_mouse,
                              nRF5x_USB_INTERFACE_MOUSE,
                              NRF_DRV_USBD_EPIN1,
                              CONFIG_MOUSE_BUTTON_COUNT,
                              nRF5x_USB_mouse_ev_handler
);


//-----------------------------------------------------------------------------
void nRF5x_USB_mouse_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_hid_user_event_t event)
{
  
}


//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_MOUSE_init(void) {
  
  nRF5x_USB_init();
  
  if(nRF5x_USB_connected()) nRF5x_USB_stop();
  
  app_usbd_class_inst_t const * class_inst_mouse;
  class_inst_mouse = app_usbd_hid_mouse_class_inst_get(&m_app_hid_mouse);
  app_usbd_class_append(class_inst_mouse);
  
  nRF5x_USB_start();
  
  return true;
}
