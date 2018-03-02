#include "nRF5x_USB_MSC.h"

#define nRF5x_USB_MSC_WORKBUFFER_SIZE   1024
#define nRF5x_USB_RAM_BLOCK_DEVICE_SIZE (256 * 512)

static uint8_t m_block_dev_ram_buff[nRF5x_USB_RAM_BLOCK_DEVICE_SIZE];


//-----------------------------------------------------------------------------
static void nRF5x_USB_msc_ev_handler(app_usbd_class_inst_t const * p_inst,
                                     app_usbd_msc_user_event_t     event);

//-----------------------------------------------------------------------------
NRF_BLOCK_DEV_RAM_DEFINE(
    m_block_dev_ram,
    NRF_BLOCK_DEV_RAM_CONFIG(512, m_block_dev_ram_buff, sizeof(m_block_dev_ram_buff)),
    NFR_BLOCK_DEV_INFO_CONFIG("Vega", "Absolute", "1.00")
);

#define BLOCKDEV_LIST() (                                   \
    NRF_BLOCKDEV_BASE_ADDR(m_block_dev_ram,   block_dev)    \
)

#define ENDPOINT_LIST() APP_USBD_MSC_ENDPOINT_LIST(1, 1)

APP_USBD_MSC_GLOBAL_DEF(m_app_msc,
                        0,
                        nRF5x_USB_msc_ev_handler,
                        ENDPOINT_LIST(),
                        BLOCKDEV_LIST(),
                        nRF5x_USB_MSC_WORKBUFFER_SIZE);

//-----------------------------------------------------------------------------
void nRF5x_USB_msc_ev_handler(app_usbd_class_inst_t const * p_inst,
                              app_usbd_msc_user_event_t     event)
{
}

//-----------------------------------------------------------------------------
uint8_t nRF5x_USB_MSC_init(void) {
  
  /* Fill whole RAM block device buffer */
  //for (size_t i = 0; i < sizeof(m_block_dev_ram_buff); ++i) {
  //    m_block_dev_ram_buff[i] = i;
  //}

  nRF5x_USB_init();
  
  if(nRF5x_USB_connected()) nRF5x_USB_stop();
  
  app_usbd_class_inst_t const * class_inst_msc;
  class_inst_msc = app_usbd_msc_class_inst_get(&m_app_msc);
  app_usbd_class_append(class_inst_msc);
    
  nRF5x_USB_start();
  
  return true;
	
}


