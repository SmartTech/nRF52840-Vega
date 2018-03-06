#include "nRF5x_USB_GENERIC.h"


#define nRF5x_USB_HID_GENERIC_EPIN      NRF_DRV_USBD_EPIN1

#define nRF5x_HID_GENERIC_EP_COUNT      1

#define nRF5x_REPORT_IN_QUEUE_SIZE      1
#define nRF5x_REPORT_OUT_MAXSIZE        0

#define ENDPOINT_LIST()                                      \
(                                                            \
        nRF5x_USB_HID_GENERIC_EPIN                           \
)

//-----------------------------------------------------------------------------
#if nRF5x_USB_USE_GENERIC
static const uint8_t m_hid_generic_rep_descriptor[] = 
        APP_USBD_HID_MOUSE_REPORT_DSC_BUTTON(2);

static void nRF5x_USB_generic_ev_handler(app_usbd_class_inst_t const * p_inst,
                                         app_usbd_hid_user_event_t event);

//-----------------------------------------------------------------------------

static const uint8_t m_hid_generic_class_descriptors[] = {
        APP_USBD_HID_GENERIC_INTERFACE_DSC(nRF5x_USB_INTERFACE_GENERIC,
                                           nRF5x_HID_GENERIC_EP_COUNT,
                                           APP_USBD_HID_SUBCLASS_BOOT,
                                           APP_USBD_HID_PROTO_GENERIC)
        APP_USBD_HID_GENERIC_HID_DSC(m_hid_generic_rep_descriptor)
        APP_USBD_HID_GENERIC_EP_DSC(nRF5x_USB_HID_GENERIC_EPIN)
};


APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_generic,
                                nRF5x_USB_INTERFACE_GENERIC,
                                nRF5x_USB_generic_ev_handler,
                                ENDPOINT_LIST(),
                                m_hid_generic_class_descriptors,
                                m_hid_generic_rep_descriptor,
                                nRF5x_REPORT_IN_QUEUE_SIZE,
                                nRF5x_REPORT_OUT_MAXSIZE);

#endif

#define HID_BTN_LEFT_MASK  (1U << 0)
#define HID_BTN_RIGHT_MASK (1U << 1)
/* HID report layout */
#define HID_BTN_IDX   0 /**< Button bit mask position */
#define HID_X_IDX     1 /**< X offset position */
#define HID_Y_IDX     2 /**< Y offset position */
#define HID_W_IDX     3 /**< Wheel position  */
#define HID_REP_SIZE  4 /**< The size of the report */

struct {
    int16_t acc_x;    /**< Accumulated x state */
    int16_t acc_y;    /**< Accumulated y state */
    uint8_t btn;      /**< Current btn state */
    uint8_t last_btn; /**< Last transfered button state */
} m_mouse_state;

static bool m_report_pending;

typedef enum {
    HID_GENERIC_MOUSE_X,
    HID_GENERIC_MOUSE_Y,
    HID_GENERIC_MOUSE_BTN_LEFT,
    HID_GENERIC_MOUSE_BTN_RIGHT,
} hid_generic_mouse_action_t;

static int8_t hid_acc_for_report_get(int16_t acc) {
         if(acc > INT8_MAX) return INT8_MAX;
    else if(acc < INT8_MIN) return INT8_MIN;
    else                    return (int8_t)(acc);
}

static void hid_generic_mouse_process_state(void) {
  if (m_report_pending) return;
  if ((m_mouse_state.acc_x != 0) ||
      (m_mouse_state.acc_y != 0) ||
      (m_mouse_state.btn != m_mouse_state.last_btn))
  {
    ret_code_t ret;
    static uint8_t report[HID_REP_SIZE];
    /* We have some status changed that we need to transfer */
    report[HID_BTN_IDX] = m_mouse_state.btn;
    report[HID_X_IDX]   = (uint8_t)hid_acc_for_report_get(m_mouse_state.acc_x);
    report[HID_Y_IDX]   = (uint8_t)hid_acc_for_report_get(m_mouse_state.acc_y);
    /* Start the transfer */
    ret = app_usbd_hid_generic_in_report_set(&m_app_hid_generic, report, sizeof(report));
    if (ret == NRF_SUCCESS) {
        m_report_pending = true;
        m_mouse_state.last_btn = report[HID_BTN_IDX];
        CRITICAL_REGION_ENTER();
        /* This part of the code can fail if interrupted by BSP keys processing.
         * Lock interrupts to be safe */
        m_mouse_state.acc_x   -= (int8_t)report[HID_X_IDX];
        m_mouse_state.acc_y   -= (int8_t)report[HID_Y_IDX];
        CRITICAL_REGION_EXIT();
    }
  }
}

//-----------------------------------------------------------------------------
void nRF5x_USB_generic_ev_handler(app_usbd_class_inst_t const * p_inst,
                                  app_usbd_hid_user_event_t event)
{
      switch (event) {
        case APP_USBD_HID_USER_EVT_OUT_REPORT_READY: {
            /* No output report defined for this example.*/
            ASSERT(0);
            break;
        }
        case APP_USBD_HID_USER_EVT_IN_REPORT_DONE: {
            m_report_pending = false;
            hid_generic_mouse_process_state();
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO: {
            //NRF_LOG_INFO("SET_BOOT_PROTO");
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO: {
            //NRF_LOG_INFO("SET_REPORT_PROTO");
            break;
        }
        default: break;
    }
}

static void hid_generic_mouse_action(hid_generic_mouse_action_t action, int8_t param)
{
    CRITICAL_REGION_ENTER();
    /*
     * Update mouse state
     */
    switch (action)
    {
      case HID_GENERIC_MOUSE_X:
          m_mouse_state.acc_x += param;
          break;
      case HID_GENERIC_MOUSE_Y:
          m_mouse_state.acc_y += param;
          break;
      case HID_GENERIC_MOUSE_BTN_RIGHT:
          if(param == 1) {
              m_mouse_state.btn |= HID_BTN_RIGHT_MASK;
          } else {
              m_mouse_state.btn &= ~HID_BTN_RIGHT_MASK;
          }
          break;
      case HID_GENERIC_MOUSE_BTN_LEFT:
          if(param == 1) {
              m_mouse_state.btn |= HID_BTN_LEFT_MASK;
          } else {
              m_mouse_state.btn &= ~HID_BTN_LEFT_MASK;
          }
          break;
    }
    CRITICAL_REGION_EXIT();
}

//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_GENERIC_init(void) {
  
#if nRF5x_USB_USE_GENERIC
  nRF5x_USB_init();
  
  m_report_pending = false;
  
  if(nRF5x_USB_connected()) nRF5x_USB_stop();
  
  app_usbd_class_inst_t const * class_inst_generic;
  class_inst_generic = app_usbd_hid_generic_class_inst_get(&m_app_hid_generic);
  app_usbd_class_append(class_inst_generic);
  
  nRF5x_USB_start();
#endif
  
  return true;
}

//-----------------------------------------------------------------------------
void nRF5x_USB_GENERIC_handle(void) {
  
  hid_generic_mouse_process_state();
  
}
