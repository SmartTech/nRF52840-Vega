#include "nrf_drv_twi.h"

#include "nrf_drv_common.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"

#define NRF_LOG_ERROR_STRING_GET(...)
#define NRF_LOG_HEXDUMP_DEBUG(...)
#define NRF_LOG_WARNING(...)
#define NRF_LOG_DEBUG(...)
#define NRF_LOG_INFO(...)

// All interrupt flags
#define DISABLE_ALL_INT_SHORT  0xFFFFFFFF

#define SCL_PIN_INIT_CONF     ( (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) \
                              | (GPIO_PIN_CNF_DRIVE_S0D1       << GPIO_PIN_CNF_DRIVE_Pos) \
                              | (GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)  \
                              | (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) \
                              | (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos))
#define SDA_PIN_INIT_CONF        SCL_PIN_INIT_CONF

#define SDA_PIN_UNINIT_CONF   ( (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) \
                              | (GPIO_PIN_CNF_DRIVE_H0H1       << GPIO_PIN_CNF_DRIVE_Pos) \
                              | (GPIO_PIN_CNF_PULL_Disabled    << GPIO_PIN_CNF_PULL_Pos)  \
                              | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) \
                              | (GPIO_PIN_CNF_DIR_Input        << GPIO_PIN_CNF_DIR_Pos))
#define SCL_PIN_UNINIT_CONF      SDA_PIN_UNINIT_CONF

#define SCL_PIN_INIT_CONF_CLR ( (GPIO_PIN_CNF_SENSE_Disabled   << GPIO_PIN_CNF_SENSE_Pos) \
                              | (GPIO_PIN_CNF_DRIVE_S0D1       << GPIO_PIN_CNF_DRIVE_Pos) \
                              | (GPIO_PIN_CNF_PULL_Pullup      << GPIO_PIN_CNF_PULL_Pos)  \
                              | (GPIO_PIN_CNF_INPUT_Connect    << GPIO_PIN_CNF_INPUT_Pos) \
                              | (GPIO_PIN_CNF_DIR_Output       << GPIO_PIN_CNF_DIR_Pos))
#define SDA_PIN_INIT_CONF_CLR    SCL_PIN_INIT_CONF_CLR

#define HW_TIMEOUT            10000

nrf_drv_twi_t nrf_drv_twi0 = NRF_DRV_TWI_INSTANCE(0);
nrf_drv_twi_t nrf_drv_twi1 = NRF_DRV_TWI_INSTANCE(1);

//-----------------------------------------------------------------------------
//
typedef struct {
    nrf_drv_twi_evt_handler_t handler;
    void *                    p_context;
    volatile uint32_t         int_mask;
    nrf_drv_twi_xfer_desc_t   xfer_desc;
    uint32_t                  flags;
    uint8_t *                 p_curr_buf;
    uint8_t                   curr_length;
    bool                      curr_no_stop;
    nrf_drv_state_t           state;
    bool                      error;
    volatile bool             busy;
    bool                      repeated;
    uint8_t                   bytes_transferred;
    bool                      hold_bus_uninit;
} twi_control_block_t;

static twi_control_block_t m_cb[TWI_INSTANCE_COUNT];

//-----------------------------------------------------------------------------
//
nrf_drv_twi_t* nrf_drv_twi_get_instance(uint8_t instance)
{
  switch(instance) {
    case 1  : return &nrf_drv_twi1;
    default : return &nrf_drv_twi0;
  }
}

//-----------------------------------------------------------------------------
uint32_t twi_process_error(uint32_t errorsrc)
{
    uint32_t ret = NRF_ERROR_INTERNAL;
    if (errorsrc & NRF_TWI_ERROR_ADDRESS_NACK) {
        ret = NRF_ERROR_DRV_TWI_ERR_ANACK;
    }
    if (errorsrc & NRF_TWI_ERROR_DATA_NACK) {
        ret = NRF_ERROR_DRV_TWI_ERR_DNACK;
    }
    return ret;
}

//-----------------------------------------------------------------------------
void twi_clear_bus(nrf_drv_twi_config_t const * p_config)
{
    NRF_GPIO->PIN_CNF[p_config->scl] = SCL_PIN_INIT_CONF;
    NRF_GPIO->PIN_CNF[p_config->sda] = SDA_PIN_INIT_CONF;

    nrf_gpio_pin_set(p_config->scl);
    nrf_gpio_pin_set(p_config->sda);

    NRF_GPIO->PIN_CNF[p_config->scl] = SCL_PIN_INIT_CONF_CLR;
    NRF_GPIO->PIN_CNF[p_config->sda] = SDA_PIN_INIT_CONF_CLR;

    nrf_delay_us(4);

    for (int i = 0; i < 9; i++) {
        if (nrf_gpio_pin_read(p_config->sda)) {
            if (i == 0) return;
            else break;
        }
        nrf_gpio_pin_clear(p_config->scl);
        nrf_delay_us(4);
        nrf_gpio_pin_set(p_config->scl);
        nrf_delay_us(4);
    }
    nrf_gpio_pin_clear(p_config->sda);
    nrf_delay_us(4);
    nrf_gpio_pin_set(p_config->sda);
}

//-----------------------------------------------------------------------------
uint8_t nrf_drv_twi_init(nrf_drv_twi_t const *        p_instance,
                         nrf_drv_twi_config_t const * p_config,
                         nrf_drv_twi_evt_handler_t    event_handler,
                         void *                       p_context)
{
    ASSERT(p_config);
    ASSERT(p_config->scl != p_config->sda);
    twi_control_block_t * p_cb  = &m_cb[p_instance->drv_inst_idx];
    uint8_t err_code;

    if (p_cb->state != NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        NRF_LOG_WARNING("Function: %s, error code: %s.",
                        (uint32_t)__func__,
                        (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    p_cb->handler         = event_handler;
    p_cb->p_context       = p_context;
    p_cb->int_mask        = 0;
    p_cb->repeated        = false;
    p_cb->busy            = false;
    p_cb->hold_bus_uninit = p_config->hold_bus_uninit;

    if (p_config->clear_bus_init)
    {
        /* Send clocks (max 9) until slave device back from stuck mode */
        twi_clear_bus(p_config);
    }

    /* To secure correct signal levels on the pins used by the TWI
       master when the system is in OFF mode, and when the TWI master is
       disabled, these pins must be configured in the GPIO peripheral.
    */
    NRF_GPIO->PIN_CNF[p_config->scl] = SCL_PIN_INIT_CONF;
    NRF_GPIO->PIN_CNF[p_config->sda] = SDA_PIN_INIT_CONF;

    NRF_TWIM_Type * p_twim = p_instance->reg.p_twim;
    nrf_twim_pins_set(p_twim, p_config->scl, p_config->sda);
    nrf_twim_frequency_set(p_twim,
        (nrf_twim_frequency_t)p_config->frequency);

    if (p_cb->handler) {
      nrf_drv_common_irq_enable(nrf_drv_get_IRQn((void *)p_instance->reg.p_twim),
          p_config->interrupt_priority);
    }

    p_cb->state = NRF_DRV_STATE_INITIALIZED;

    err_code = NRF_SUCCESS;
    NRF_LOG_INFO("Function: %s, error code: %s.",
                 (uint32_t)__func__,
                 (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

//-----------------------------------------------------------------------------
void nrf_drv_twi_uninit(nrf_drv_twi_t const * p_instance)
{
    twi_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    ASSERT(p_cb->state != NRF_DRV_STATE_UNINITIALIZED);

    if (p_cb->handler) {
        nrf_drv_common_irq_disable(nrf_drv_get_IRQn((void *)p_instance->reg.p_twim));
    }
    nrf_drv_twi_disable(p_instance);

    if (!p_cb->hold_bus_uninit) {
      NRF_GPIO->PIN_CNF[p_instance->reg.p_twim->PSEL.SCL] = SCL_PIN_UNINIT_CONF;
      NRF_GPIO->PIN_CNF[p_instance->reg.p_twim->PSEL.SDA] = SDA_PIN_UNINIT_CONF;
    }

    p_cb->state = NRF_DRV_STATE_UNINITIALIZED;
    NRF_LOG_INFO("Instance uninitialized: %d.", p_instance->drv_inst_idx);
}

//-----------------------------------------------------------------------------
void nrf_drv_twi_enable(nrf_drv_twi_t const * p_instance)
{
    twi_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    ASSERT(p_cb->state == NRF_DRV_STATE_INITIALIZED);

    NRF_TWIM_Type * p_twim = p_instance->reg.p_twim;
    nrf_twim_enable(p_twim);

    p_cb->state = NRF_DRV_STATE_POWERED_ON;
    NRF_LOG_INFO("Instance enabled: %d.", p_instance->drv_inst_idx);
}

//-----------------------------------------------------------------------------
void nrf_drv_twi_disable(nrf_drv_twi_t const * p_instance)
{
    twi_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    ASSERT(p_cb->state != NRF_DRV_STATE_UNINITIALIZED);

    NRF_TWIM_Type * p_twim = p_instance->reg.p_twim;
    p_cb->int_mask = 0;
    nrf_twim_int_disable(p_twim, DISABLE_ALL_INT_SHORT);
    nrf_twim_shorts_disable(p_twim, DISABLE_ALL_INT_SHORT);
    nrf_twim_disable(p_twim);

    p_cb->state = NRF_DRV_STATE_INITIALIZED;
    NRF_LOG_INFO("Instance disabled: %d.", p_instance->drv_inst_idx);
}

//-----------------------------------------------------------------------------
bool nrf_drv_twi_is_busy(nrf_drv_twi_t const * p_instance)
{
    twi_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    return p_cb->busy;
}

//-----------------------------------------------------------------------------
void twim_list_enable_handle(NRF_TWIM_Type * p_twim, uint32_t flags)
{
    if (NRF_DRV_TWI_FLAG_TX_POSTINC & flags) {
        nrf_twim_tx_list_enable(p_twim);
    } else {
        nrf_twim_tx_list_disable(p_twim);
    }

    if (NRF_DRV_TWI_FLAG_RX_POSTINC & flags) {
        nrf_twim_rx_list_enable(p_twim);
    } else {
        nrf_twim_rx_list_disable(p_twim);
    }
}

//-----------------------------------------------------------------------------
uint8_t twim_xfer(twi_control_block_t           * p_cb,
                     NRF_TWIM_Type                 * p_twim,
                     nrf_drv_twi_xfer_desc_t const * p_xfer_desc,
                     uint32_t                        flags)
{
    uint8_t err_code = NRF_SUCCESS;
    nrf_twim_task_t  start_task = NRF_TWIM_TASK_STARTTX;
    nrf_twim_event_t evt_to_wait = NRF_TWIM_EVENT_STOPPED;

    if (!nrf_drv_is_in_RAM(p_xfer_desc->p_primary_buf))
    {
        err_code = NRF_ERROR_INVALID_ADDR;
        NRF_LOG_WARNING("Function: %s, error code: %s.",
                        (uint32_t)__func__,
                        (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    /* Block TWI interrupts to ensure that function is not interrupted by TWI interrupt. */
    nrf_twim_int_disable(p_twim, DISABLE_ALL_INT_SHORT);
    if (p_cb->busy)
    {
        nrf_twim_int_enable(p_twim, p_cb->int_mask);
        err_code = NRF_ERROR_BUSY;
        NRF_LOG_WARNING("Function: %s, error code: %s.",
                        (uint32_t)__func__,
                        (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    else
    {
        p_cb->busy = ((NRF_DRV_TWI_FLAG_NO_XFER_EVT_HANDLER & flags) ||
                      (NRF_DRV_TWI_FLAG_REPEATED_XFER & flags)) ? false: true;
    }

    p_cb->xfer_desc = *p_xfer_desc;
    p_cb->repeated = (flags & NRF_DRV_TWI_FLAG_REPEATED_XFER) ? true : false;
    nrf_twim_address_set(p_twim, p_xfer_desc->address);

    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_STOPPED);
    nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_ERROR);

    twim_list_enable_handle(p_twim, flags);
    switch (p_xfer_desc->type)
    {
    case NRF_DRV_TWI_XFER_TXTX:
        ASSERT(!(flags & NRF_DRV_TWI_FLAG_REPEATED_XFER));
        ASSERT(!(flags & NRF_DRV_TWI_FLAG_HOLD_XFER));
        ASSERT(!(flags & NRF_DRV_TWI_FLAG_NO_XFER_EVT_HANDLER));
        if (!nrf_drv_is_in_RAM(p_xfer_desc->p_secondary_buf))
        {
            err_code = NRF_ERROR_INVALID_ADDR;
            NRF_LOG_WARNING("Function: %s, error code: %s.",
                            (uint32_t)__func__,
                            (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
        nrf_twim_shorts_set(p_twim, NRF_TWIM_SHORT_LASTTX_SUSPEND_MASK);
        nrf_twim_tx_buffer_set(p_twim, p_xfer_desc->p_primary_buf, p_xfer_desc->primary_length);
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_TXSTARTED);
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_LASTTX);
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_SUSPENDED);
        nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_RESUME);
        nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_STARTTX);
        while (!nrf_twim_event_check(p_twim, NRF_TWIM_EVENT_TXSTARTED))
        {}
        NRF_LOG_DEBUG("TWIM: Event: %s.", (uint32_t)EVT_TO_STR_TWIM(NRF_TWIM_EVENT_TXSTARTED));
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_TXSTARTED);
        nrf_twim_tx_buffer_set(p_twim, p_xfer_desc->p_secondary_buf, p_xfer_desc->secondary_length);
        p_cb->int_mask = NRF_TWIM_INT_SUSPENDED_MASK | NRF_TWIM_INT_ERROR_MASK;
        break;
    case NRF_DRV_TWI_XFER_TXRX:
        nrf_twim_tx_buffer_set(p_twim, p_xfer_desc->p_primary_buf, p_xfer_desc->primary_length);
        if (!nrf_drv_is_in_RAM(p_xfer_desc->p_secondary_buf))
        {
            err_code = NRF_ERROR_INVALID_ADDR;
            NRF_LOG_WARNING("Function: %s, error code: %s.",
                            (uint32_t)__func__,
                            (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
            return err_code;
        }
        nrf_twim_rx_buffer_set(p_twim, p_xfer_desc->p_secondary_buf, p_xfer_desc->secondary_length);
        nrf_twim_shorts_set(p_twim, NRF_TWIM_SHORT_LASTTX_STARTRX_MASK |
                                    NRF_TWIM_SHORT_LASTRX_STOP_MASK);
        p_cb->int_mask = NRF_TWIM_INT_STOPPED_MASK | NRF_TWIM_INT_ERROR_MASK;
        break;
    case NRF_DRV_TWI_XFER_TX:
        nrf_twim_tx_buffer_set(p_twim, p_xfer_desc->p_primary_buf, p_xfer_desc->primary_length);
        if (NRF_DRV_TWI_FLAG_TX_NO_STOP & flags)
        {
            nrf_twim_shorts_set(p_twim, NRF_TWIM_SHORT_LASTTX_SUSPEND_MASK);
            p_cb->int_mask = NRF_TWIM_INT_SUSPENDED_MASK | NRF_TWIM_INT_ERROR_MASK;
            nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_SUSPENDED);
            evt_to_wait = NRF_TWIM_EVENT_SUSPENDED;
        }
        else
        {
            nrf_twim_shorts_set(p_twim, NRF_TWIM_SHORT_LASTTX_STOP_MASK);
            p_cb->int_mask = NRF_TWIM_INT_STOPPED_MASK | NRF_TWIM_INT_ERROR_MASK;
        }
        nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_RESUME);
        break;
    case NRF_DRV_TWI_XFER_RX:
        nrf_twim_rx_buffer_set(p_twim, p_xfer_desc->p_primary_buf, p_xfer_desc->primary_length);
        nrf_twim_shorts_set(p_twim, NRF_TWIM_SHORT_LASTRX_STOP_MASK);
        p_cb->int_mask = NRF_TWIM_INT_STOPPED_MASK | NRF_TWIM_INT_ERROR_MASK;
        start_task = NRF_TWIM_TASK_STARTRX;
        nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_RESUME);
        break;
    default:
        err_code = NRF_ERROR_INVALID_PARAM;
        break;
    }

    if (!(flags & NRF_DRV_TWI_FLAG_HOLD_XFER) && (p_xfer_desc->type != NRF_DRV_TWI_XFER_TXTX))
    {
        nrf_twim_task_trigger(p_twim, start_task);
    }

    if (p_cb->handler) {
        if (flags & NRF_DRV_TWI_FLAG_NO_XFER_EVT_HANDLER) {
            p_cb->int_mask = NRF_TWIM_INT_ERROR_MASK;
        }
        nrf_twim_int_enable(p_twim, p_cb->int_mask);
    }
    else
    {
        while (!nrf_twim_event_check(p_twim, evt_to_wait))
        {
            if (nrf_twim_event_check(p_twim, NRF_TWIM_EVENT_ERROR))
            {
                NRF_LOG_DEBUG("TWIM: Event: %s.",
                              (uint32_t)EVT_TO_STR_TWIM(NRF_TWIM_EVENT_ERROR));
                nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_ERROR);
                nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_RESUME);
                nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_STOP);
                evt_to_wait = NRF_TWIM_EVENT_STOPPED;
            }
        }

        uint32_t errorsrc =  nrf_twim_errorsrc_get_and_clear(p_twim);

        p_cb->busy = false;

        if (errorsrc)
        {
            err_code = twi_process_error(errorsrc);
        }
    }
    return err_code;
}

//-----------------------------------------------------------------------------
uint8_t nrf_drv_twi_xfer(nrf_drv_twi_t           const * p_instance,
                            nrf_drv_twi_xfer_desc_t const * p_xfer_desc,
                            uint32_t                        flags)
{

    uint8_t err_code = NRF_SUCCESS;
    twi_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    // TXRX and TXTX transfers are support only in non-blocking mode.
    ASSERT( !((p_cb->handler == NULL) && (p_xfer_desc->type == NRF_DRV_TWI_XFER_TXRX)));
    ASSERT( !((p_cb->handler == NULL) && (p_xfer_desc->type == NRF_DRV_TWI_XFER_TXTX)));

    NRF_LOG_INFO("Transfer type: %s.", (uint32_t)TRANSFER_TO_STR(p_xfer_desc->type));
    NRF_LOG_INFO("Transfer buffers length: primary: %d, secondary: %d.",
                 p_xfer_desc->primary_length, p_xfer_desc->secondary_length);
    NRF_LOG_DEBUG("Primary buffer data:");
    NRF_LOG_HEXDUMP_DEBUG((uint8_t *)p_xfer_desc->p_primary_buf,
                          p_xfer_desc->primary_length * sizeof(p_xfer_desc->p_primary_buf[0]));
    NRF_LOG_DEBUG("Secondary buffer data:");
    NRF_LOG_HEXDUMP_DEBUG((uint8_t *)p_xfer_desc->p_secondary_buf,
                          p_xfer_desc->secondary_length * sizeof(p_xfer_desc->p_secondary_buf[0]));

    err_code = twim_xfer(p_cb, (NRF_TWIM_Type *)p_instance->reg.p_twim, p_xfer_desc, flags);

    NRF_LOG_WARNING("Function: %s, error code: %s.",
                    (uint32_t)__func__, (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

//-----------------------------------------------------------------------------
uint8_t nrf_drv_twi_tx(nrf_drv_twi_t const * p_instance,
                          uint8_t               address,
                          uint8_t const *       p_data,
                          uint8_t               length,
                          bool                  no_stop)
{
    nrf_drv_twi_xfer_desc_t xfer = NRF_DRV_TWI_XFER_DESC_TX(address, (uint8_t*)p_data, length);
    return nrf_drv_twi_xfer(p_instance, &xfer, no_stop ? NRF_DRV_TWI_FLAG_TX_NO_STOP : 0);
}

//-----------------------------------------------------------------------------
uint8_t nrf_drv_twi_rx(nrf_drv_twi_t const * p_instance,
                          uint8_t               address,
                          uint8_t *             p_data,
                          uint8_t               length)
{
    nrf_drv_twi_xfer_desc_t xfer = NRF_DRV_TWI_XFER_DESC_RX(address, p_data, length);
    return nrf_drv_twi_xfer(p_instance, &xfer, 0);
}

//-----------------------------------------------------------------------------
uint32_t nrf_drv_twi_data_count_get(nrf_drv_twi_t const * const p_instance)
{
    ASSERT(false);
    return 0;
}

//-----------------------------------------------------------------------------
uint32_t nrf_drv_twi_start_task_get(nrf_drv_twi_t const * p_instance,
                                    nrf_drv_twi_xfer_type_t xfer_type)
{
    return (uint32_t)nrf_twim_task_address_get(p_instance->reg.p_twim,
           (xfer_type != NRF_DRV_TWI_XFER_RX) ? NRF_TWIM_TASK_STARTTX : NRF_TWIM_TASK_STARTRX);
}

//-----------------------------------------------------------------------------
uint32_t nrf_drv_twi_stopped_event_get(nrf_drv_twi_t const * p_instance)
{
    return (uint32_t)nrf_twim_event_address_get(p_instance->reg.p_twim, NRF_TWIM_EVENT_STOPPED);
}

//-----------------------------------------------------------------------------
static void irq_handler_twim(NRF_TWIM_Type * p_twim, twi_control_block_t * p_cb)
{

    ASSERT(p_cb->handler);

    if (nrf_twim_event_check(p_twim, NRF_TWIM_EVENT_ERROR))
    {
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_ERROR);
        NRF_LOG_DEBUG("TWIM: Event: %s.", (uint32_t)EVT_TO_STR_TWIM(NRF_TWIM_EVENT_ERROR));
        if (!nrf_twim_event_check(p_twim, NRF_TWIM_EVENT_STOPPED))
        {
            nrf_twim_int_disable(p_twim, p_cb->int_mask);
            p_cb->int_mask = NRF_TWIM_INT_STOPPED_MASK;
            nrf_twim_int_enable(p_twim, p_cb->int_mask);

            nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_RESUME);
            nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_STOP);
            return;
        }
    }

    nrf_drv_twi_evt_t event;

    if (nrf_twim_event_check(p_twim, NRF_TWIM_EVENT_STOPPED))
    {
        NRF_LOG_DEBUG("TWIM: Event: %s.", (uint32_t)EVT_TO_STR_TWIM(NRF_TWIM_EVENT_STOPPED));
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_STOPPED);
        event.xfer_desc = p_cb->xfer_desc;
        if (p_cb->error)
        {

            event.xfer_desc.primary_length = (p_cb->xfer_desc.type == NRF_DRV_TWI_XFER_RX) ?
                (uint8_t)nrf_twim_rxd_amount_get(p_twim) : (uint8_t)nrf_twim_txd_amount_get(p_twim);
            event.xfer_desc.secondary_length = (p_cb->xfer_desc.type == NRF_DRV_TWI_XFER_TXRX) ?
                (uint8_t)nrf_twim_rxd_amount_get(p_twim) : (uint8_t)nrf_twim_txd_amount_get(p_twim);

        }
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_LASTTX);
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_LASTRX);
        if (!p_cb->repeated || p_cb->error)
        {
            nrf_twim_shorts_set(p_twim, 0);
            p_cb->int_mask = 0;
            nrf_twim_int_disable(p_twim, DISABLE_ALL_INT_SHORT);
        }
    }
    else
    {
        nrf_twim_event_clear(p_twim, NRF_TWIM_EVENT_SUSPENDED);
        NRF_LOG_DEBUG("TWIM: Event: %s.", (uint32_t)EVT_TO_STR_TWIM(NRF_TWIM_EVENT_SUSPENDED));
        if (p_cb->xfer_desc.type == NRF_DRV_TWI_XFER_TX)
        {
            event.xfer_desc = p_cb->xfer_desc;
            if (!p_cb->repeated)
            {
                nrf_twim_shorts_set(p_twim, 0);
                p_cb->int_mask = 0;
                nrf_twim_int_disable(p_twim, DISABLE_ALL_INT_SHORT);
            }
        }
        else
        {
            nrf_twim_shorts_set(p_twim, NRF_TWIM_SHORT_LASTTX_STOP_MASK);
            p_cb->int_mask = NRF_TWIM_INT_STOPPED_MASK | NRF_TWIM_INT_ERROR_MASK;
            nrf_twim_int_disable(p_twim, DISABLE_ALL_INT_SHORT);
            nrf_twim_int_enable(p_twim, p_cb->int_mask);
            nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_STARTTX);
            nrf_twim_task_trigger(p_twim, NRF_TWIM_TASK_RESUME);
            return;
        }
    }

    uint32_t errorsrc = nrf_twim_errorsrc_get_and_clear(p_twim);
    if (errorsrc & NRF_TWIM_ERROR_ADDRESS_NACK)
    {
        event.type = NRF_DRV_TWI_EVT_ADDRESS_NACK;
        NRF_LOG_DEBUG("Event: %s.", (uint32_t)EVT_TO_STR(NRF_DRV_TWI_EVT_ADDRESS_NACK));
    }
    else if (errorsrc & NRF_TWIM_ERROR_DATA_NACK)
    {
        event.type = NRF_DRV_TWI_EVT_DATA_NACK;
        NRF_LOG_DEBUG("Event: %s.", (uint32_t)EVT_TO_STR(NRF_DRV_TWI_EVT_DATA_NACK));
    }
    else
    {
        event.type = NRF_DRV_TWI_EVT_DONE;
        NRF_LOG_DEBUG("Event: %s.", (uint32_t)EVT_TO_STR(NRF_DRV_TWI_EVT_DONE));
    }

    if (!p_cb->repeated)
    {
        p_cb->busy = false;
    }
    p_cb->handler(&event, p_cb->p_context);
}

void TWI0_IRQ_HANDLER(void) {
  irq_handler_twim(NRF_TWIM0, &m_cb[TWI0_INSTANCE_INDEX]);
}

void TWI1_IRQ_HANDLER(void) {
  irq_handler_twim(NRF_TWIM1, &m_cb[TWI1_INSTANCE_INDEX]);
}

