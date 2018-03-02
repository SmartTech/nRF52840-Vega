#include "nRF5x_USB_AUDIO.h"


#define HP_INTERFACES_CONFIG()  APP_USBD_AUDIO_CONFIG_OUT(0, 1)
#define MIC_INTERFACES_CONFIG() APP_USBD_AUDIO_CONFIG_IN(2, 3)

#define BUFFER_SIZE                      48

#define HP_TERMINAL_CH_CONFIG()                                                                       \
        (APP_USBD_AUDIO_IN_TERM_CH_CONFIG_LEFT_FRONT | APP_USBD_AUDIO_IN_TERM_CH_CONFIG_RIGHT_FRONT)

/**
 * @brief   Feature controls
 *      channel 0
 *      channel 1
 */
#define HP_FEATURE_CONTROLS()                                                                         \
        APP_USBD_U16_TO_RAW_DSC(APP_USBD_AUDIO_FEATURE_UNIT_CONTROL_MUTE),                         \
        APP_USBD_U16_TO_RAW_DSC(APP_USBD_AUDIO_FEATURE_UNIT_CONTROL_MUTE),                         \
        APP_USBD_U16_TO_RAW_DSC(APP_USBD_AUDIO_FEATURE_UNIT_CONTROL_MUTE)
/**
 * @brief Audio control interface descriptor list:
 *  - input terminal 1
 *  - feature unit descriptor
 *  - output terminal
 */
#define HP_AUDIO_CONTROL_DSC_LIST() (                                                                           \
        APP_USBD_AUDIO_INPUT_TERMINAL_DSC(1, APP_USBD_AUDIO_TERMINAL_USB_STREAMING, 2, HP_TERMINAL_CH_CONFIG()) \
        APP_USBD_AUDIO_FEATURE_UNIT_DSC(2, 1, HP_FEATURE_CONTROLS())                                            \
        APP_USBD_AUDIO_OUTPUT_TERMINAL_DSC(3, APP_USBD_AUDIO_TERMINAL_OUT_HEADPHONES, 2)                     \
)

#define MIC_TERMINAL_CH_CONFIG()                                                                       \
        (APP_USBD_AUDIO_IN_TERM_CH_CONFIG_LEFT_FRONT | APP_USBD_AUDIO_IN_TERM_CH_CONFIG_RIGHT_FRONT)

/**
 * @brief   Feature controls
 *      channel 0
 *      channel 1
 */
#define MIC_FEATURE_CONTROLS()                                                                     \
        APP_USBD_U16_TO_RAW_DSC(APP_USBD_AUDIO_FEATURE_UNIT_CONTROL_MUTE),                         \
        APP_USBD_U16_TO_RAW_DSC(APP_USBD_AUDIO_FEATURE_UNIT_CONTROL_MUTE),                         \
        APP_USBD_U16_TO_RAW_DSC(APP_USBD_AUDIO_FEATURE_UNIT_CONTROL_MUTE)

/**
 * @brief Audio control interface descriptor list:
 *  - input terminal 1
 *  - feature unit descriptor
 *  - output terminal
 */
#define MIC_AUDIO_CONTROL_DSC_LIST() (                                                                           \
        APP_USBD_AUDIO_INPUT_TERMINAL_DSC(1, APP_USBD_AUDIO_TERMINAL_IN_MICROPHONE, 2, MIC_TERMINAL_CH_CONFIG()) \
        APP_USBD_AUDIO_FEATURE_UNIT_DSC(2, 1, MIC_FEATURE_CONTROLS())                                            \
        APP_USBD_AUDIO_OUTPUT_TERMINAL_DSC(3, APP_USBD_AUDIO_TERMINAL_USB_STREAMING, 2)                          \
)

static const uint8_t m_hp_audio_class_descriptors[] = {
    /* Audio control interface descriptor descriptor: 0
     * + descriptors defined by AUDIO_CONTROL_DSC_LIST*/
    APP_USBD_AUDIO_CONTROL_DSC(0, HP_AUDIO_CONTROL_DSC_LIST(), (1))

    /*Audio streaming interface descriptor descriptor: 1, setting: 0*/
    APP_USBD_AUDIO_STREAMING_DSC(1, 0, 0)

    /*Audio streaming interface descriptor descriptor: 1, setting: 1*/
    APP_USBD_AUDIO_STREAMING_DSC(1, 1, 1)

    /*Audio class specific interface descriptor*/
    APP_USBD_AUDIO_AS_IFACE_DSC(1, 0, APP_USBD_AUDIO_AS_IFACE_FORMAT_PCM)

    /*Audio class specific format I descriptor*/
    APP_USBD_AUDIO_AS_FORMAT_III_DSC(2, 2, 16, 1, APP_USBD_U24_TO_RAW_DSC(48000))

    /*Audio class specific endpoint general descriptor*/
    APP_USBD_AUDIO_EP_GENERAL_DSC(0x00, 0, 0)

    /*Standard ISO endpoint descriptor*/
    APP_USBD_AUDIO_ISO_EP_OUT_DSC(192)
};

static const uint8_t m_mic_audio_class_descriptors[] = {
    /* Audio control interface descriptor descriptor: 2
     * + descriptors defined by AUDIO_CONTROL_DSC_LIST*/
    APP_USBD_AUDIO_CONTROL_DSC(2, MIC_AUDIO_CONTROL_DSC_LIST(), (3))

    /*Audio streaming interface descriptor descriptor: 3, setting: 0*/
    APP_USBD_AUDIO_STREAMING_DSC(3, 0, 0)

    /*Audio streaming interface descriptor descriptor: 3, setting: 1*/
    APP_USBD_AUDIO_STREAMING_DSC(3, 1, 1)

    /*Audio class specific interface descriptor*/
    APP_USBD_AUDIO_AS_IFACE_DSC(3, 0, APP_USBD_AUDIO_AS_IFACE_FORMAT_PCM)
      
    /*Audio class specific format I descriptor*/
    APP_USBD_AUDIO_AS_FORMAT_I_DSC(2, 2, 16, 1, APP_USBD_U24_TO_RAW_DSC(48000))

    /*Audio class specific endpoint general descriptor*/
    APP_USBD_AUDIO_EP_GENERAL_DSC(0x00, 0, 0)

    /*Standard ISO endpoint descriptor*/
    APP_USBD_AUDIO_ISO_EP_IN_DSC(192)
};


static void nRF5x_USB_headphone_ev_handler(app_usbd_class_inst_t const * p_inst,
                                           app_usbd_audio_user_event_t   event);

static void nRF5x_USB_microphone_ev_handler(app_usbd_class_inst_t const * p_inst,
                                            app_usbd_audio_user_event_t   event);

static size_t   m_temp_buffer_size;
static int16_t  m_temp_buffer[2 * BUFFER_SIZE];
static uint8_t  m_mute_hp;
static uint32_t m_freq_hp;
static uint8_t  m_mute_mic;
static uint32_t m_freq_mic;

APP_USBD_AUDIO_GLOBAL_DEF(m_app_audio_headphone,
                          HP_INTERFACES_CONFIG(),
                          nRF5x_USB_headphone_ev_handler,
                          m_hp_audio_class_descriptors
);

APP_USBD_AUDIO_GLOBAL_DEF(m_app_audio_microphone,
                          MIC_INTERFACES_CONFIG(),
                          nRF5x_USB_microphone_ev_handler,
                          m_mic_audio_class_descriptors
);


//-----------------------------------------------------------------------------
void nRF5x_USB_headphone_class_req(app_usbd_class_inst_t const * p_inst)
{
  app_usbd_audio_t const * p_audio = app_usbd_audio_class_get(p_inst);
  app_usbd_audio_req_t * p_req = app_usbd_audio_class_request_get(p_audio);
  UNUSED_VARIABLE(m_mute_hp);
  UNUSED_VARIABLE(m_freq_hp);
  switch (p_req->req_target) {
      case APP_USBD_AUDIO_CLASS_REQ_IN:
          if (p_req->req_type == APP_USBD_AUDIO_REQ_SET_CUR) {
              //Only mute control is defined
              p_req->payload[0] = m_mute_hp;
          }
          break;
      case APP_USBD_AUDIO_CLASS_REQ_OUT:
          if (p_req->req_type == APP_USBD_AUDIO_REQ_SET_CUR) {
              //Only mute control is defined
              m_mute_hp = p_req->payload[0];
          }
          break;
      case APP_USBD_AUDIO_EP_REQ_IN:  break;
      case APP_USBD_AUDIO_EP_REQ_OUT:
          if (p_req->req_type == APP_USBD_AUDIO_REQ_SET_CUR) {
              //Only set frequency is supported
              m_freq_hp = uint24_decode(p_req->payload);
          }
          break;
      default: break;
  }
}

void nRF5x_USB_microphone_class_req(app_usbd_class_inst_t const * p_inst)
{
  app_usbd_audio_t const * p_audio = app_usbd_audio_class_get(p_inst);
  app_usbd_audio_req_t * p_req = app_usbd_audio_class_request_get(p_audio);
  UNUSED_VARIABLE(m_mute_mic);
  UNUSED_VARIABLE(m_freq_mic);
  switch (p_req->req_target) {
      case APP_USBD_AUDIO_CLASS_REQ_IN:
          if (p_req->req_type == APP_USBD_AUDIO_REQ_SET_CUR) {
              //Only mute control is defined
              p_req->payload[0] = m_mute_mic;
          }
          break;
      case APP_USBD_AUDIO_CLASS_REQ_OUT:
          if (p_req->req_type == APP_USBD_AUDIO_REQ_SET_CUR) {
              //Only mute control is defined
              m_mute_mic = p_req->payload[0];
          }
          break;
      case APP_USBD_AUDIO_EP_REQ_IN:  break;
      case APP_USBD_AUDIO_EP_REQ_OUT:
          if (p_req->req_type == APP_USBD_AUDIO_REQ_SET_CUR) {
              //Only set frequency is supported
              m_freq_mic = uint24_decode(p_req->payload);
          }
          break;
      default: break;
  }
}

//-----------------------------------------------------------------------------
void nRF5x_USB_headphone_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_audio_user_event_t   event)
{
    app_usbd_audio_t const * p_audio = app_usbd_audio_class_get(p_inst);
    UNUSED_VARIABLE(p_audio);
    switch (event){
        case APP_USBD_AUDIO_USER_EVT_CLASS_REQ:
            nRF5x_USB_headphone_class_req(p_inst);
            break;
        case APP_USBD_AUDIO_USER_EVT_RX_DONE:
            uint8_t ret;
            /* Block from headphones copied into buffer, send it into microphone input */
            ret = app_usbd_audio_class_tx_start(&m_app_audio_microphone.base, m_temp_buffer, m_temp_buffer_size);
            if (NRF_SUCCESS == ret){
              //
            }
            break;
        default:
            break;
    }
}

void nRF5x_USB_microphone_ev_handler(app_usbd_class_inst_t const * p_inst,
                                      app_usbd_audio_user_event_t   event)
{
    app_usbd_audio_t const * p_audio = app_usbd_audio_class_get(p_inst);
    UNUSED_VARIABLE(p_audio);
    switch (event) {
        case APP_USBD_AUDIO_USER_EVT_CLASS_REQ:
            nRF5x_USB_microphone_class_req(p_inst);
            break;
        case APP_USBD_AUDIO_USER_EVT_TX_DONE: break;
        default: break;
    }
}


//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_AUDIO_init(void) {
  
  nRF5x_USB_init();
  
  if(nRF5x_USB_connected()) nRF5x_USB_stop();
  
  app_usbd_class_inst_t const * class_inst_hp;
  class_inst_hp = app_usbd_audio_class_inst_get(&m_app_audio_headphone);
  app_usbd_class_append(class_inst_hp);
  
  app_usbd_class_inst_t const * class_inst_mic;
  class_inst_mic = app_usbd_audio_class_inst_get(&m_app_audio_microphone);
  app_usbd_class_append(class_inst_mic);
  
  nRF5x_USB_start();
  
  return true;
	
}
