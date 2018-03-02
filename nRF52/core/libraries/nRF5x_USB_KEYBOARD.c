#include "nRF5x_USB_KEYBOARD.h"


static void nRF5x_USB_kbd_ev_handler(app_usbd_class_inst_t const * p_inst,
                                     app_usbd_hid_user_event_t event);

APP_USBD_HID_KBD_GLOBAL_DEF(m_app_hid_kbd,
                            nRF5x_USB_INTERFACE_KBD,
                            NRF_DRV_USBD_EPIN2,
                            nRF5x_USB_kbd_ev_handler
);


//-----------------------------------------------------------------------------
void nRF5x_USB_kbd_ev_handler(app_usbd_class_inst_t const * p_inst,
                              app_usbd_hid_user_event_t event)
{
  
}


//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_KEYBOARD_init(void) {
  
  nRF5x_USB_init();
  
  if(nRF5x_USB_connected()) nRF5x_USB_stop();
  
  app_usbd_class_inst_t const * class_inst_kbd;
  class_inst_kbd = app_usbd_hid_kbd_class_inst_get(&m_app_hid_kbd);
  app_usbd_class_append(class_inst_kbd);

  nRF5x_USB_start();
  
  return true;
}
