#include "nrf_drv_spi.h"

#define NRF_LOG_INFO(...)
#define NRF_LOG_DEBUG(...)
#define NRF_LOG_WARNING(...)
#define NRF_LOG_HEXDUMP_DEBUG(...)
#define NRF_LOG_ERROR_STRING_GET(...)

#define END_INT_MASK     NRF_SPIM_INT_END_MASK


nrf_drv_spi_t nrf_drv_spi0 = NRF_DRV_SPI_INSTANCE(0);
nrf_drv_spi_t nrf_drv_spi1 = NRF_DRV_SPI_INSTANCE(1);
nrf_drv_spi_t nrf_drv_spi2 = NRF_DRV_SPI_INSTANCE(2);
nrf_drv_spi_t nrf_drv_spi3 = NRF_DRV_SPI_INSTANCE(3);

//-----------------------------------------------------------------------------
// Ѕлок управлени€ - локальные данные экземпл€ра драйвера
typedef struct {
    nrf_drv_spi_evt_handler_t handler;
    void *                    p_context;
    nrf_drv_spi_evt_t         evt;
    nrf_drv_state_t           state;
    volatile bool             transfer_in_progress;
    uint8_t                   ss_pin;
    uint8_t                   orc;
    uint8_t                   bytes_transferred;
    #if NRF_MODULE_ENABLED(SPIM_NRF52_ANOMALY_109_WORKAROUND)
    uint8_t                   tx_length;
    uint8_t                   rx_length;
    #endif
    bool                      tx_done : 1;
    bool                      rx_done : 1;
    bool                      abort   : 1;
} spi_control_block_t;

// ќбъ€влени€ блоков управлени€ дл€ каждого экземпл€ра драйвера
static spi_control_block_t m_cb[SPI_INSTANCE_COUNT];

//-----------------------------------------------------------------------------
//
nrf_drv_spi_t* nrf_drv_spi_get_instance(uint8_t instance)
{
  switch(instance) {
    case 0  : return &nrf_drv_spi0;
    case 1  : return &nrf_drv_spi1;
    case 2  : return &nrf_drv_spi2;
    default : return &nrf_drv_spi3;
  }
}

//-----------------------------------------------------------------------------
//
ret_code_t nrf_drv_spi_init(nrf_drv_spi_t const * const p_instance,
                            nrf_drv_spi_config_t const * p_config,
                            nrf_drv_spi_evt_handler_t handler,
                            void * p_context)
{
    ASSERT(p_config);
    spi_control_block_t * p_cb  = &m_cb[p_instance->instance_id];
    ret_code_t err_code;

    if (p_cb->state != NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        NRF_LOG_WARNING("Function: %s, error code: %s.",
                        (uint32_t)__func__,
                        (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    p_cb->handler = handler;
    p_cb->p_context = p_context;

    uint32_t mosi_pin;
    uint32_t miso_pin;
    // Configure pins used by the peripheral:
    // - SCK - output with initial value corresponding with the SPI mode used:
    //   0 - for modes 0 and 1 (CPOL = 0), 1 - for modes 2 and 3 (CPOL = 1);
    //   according to the reference manual guidelines this pin and its input
    //   buffer must always be connected for the SPI to work.
    if (p_config->mode <= NRF_DRV_SPI_MODE_1)
    {
        nrf_gpio_pin_clear(p_config->sck_pin);
    }
    else
    {
        nrf_gpio_pin_set(p_config->sck_pin);
    }
    nrf_gpio_cfg(p_config->sck_pin,
                 NRF_GPIO_PIN_DIR_OUTPUT,
                 NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_NOPULL,
                 NRF_GPIO_PIN_S0S1,
                 NRF_GPIO_PIN_NOSENSE);
    // - MOSI (optional) - output with initial value 0,
    if (p_config->mosi_pin != NRF_DRV_SPI_PIN_NOT_USED)
    {
        mosi_pin = p_config->mosi_pin;
        nrf_gpio_pin_clear(mosi_pin);
        nrf_gpio_cfg_output(mosi_pin);
    }
    else
    {
        mosi_pin = NRF_SPI_PIN_NOT_CONNECTED;
    }
    // - MISO (optional) - input,
    if (p_config->miso_pin != NRF_DRV_SPI_PIN_NOT_USED)
    {
        miso_pin = p_config->miso_pin;
        nrf_gpio_cfg_input(miso_pin, (nrf_gpio_pin_pull_t)NRF_SPI_DRV_MISO_PULL_CFG);
    }
    else
    {
        miso_pin = NRF_SPI_PIN_NOT_CONNECTED;
    }
    // - Slave Select (optional) - output with initial value 1 (inactive).
    if (p_config->ss_pin != NRF_DRV_SPI_PIN_NOT_USED)
    {
        nrf_gpio_pin_set(p_config->ss_pin);
        nrf_gpio_cfg_output(p_config->ss_pin);
    }
    m_cb[p_instance->instance_id].ss_pin = p_config->ss_pin;

    NRF_SPIM_Type * p_spim = (NRF_SPIM_Type *)p_instance->p_registers;
    nrf_spim_pins_set(p_spim, p_config->sck_pin, mosi_pin, miso_pin);
    nrf_spim_frequency_set(p_spim,
        (nrf_spim_frequency_t)p_config->frequency);
    nrf_spim_configure(p_spim,
        (nrf_spim_mode_t)p_config->mode,
        (nrf_spim_bit_order_t)p_config->bit_order);

    nrf_spim_orc_set(p_spim, p_config->orc);

    if (p_cb->handler) {
        nrf_spim_int_enable(p_spim, END_INT_MASK);
    }

    nrf_spim_enable(p_spim);

    if (p_cb->handler) {
        nrf_drv_common_irq_enable(p_instance->irq, p_config->irq_priority);
    }

    p_cb->transfer_in_progress = false;
    p_cb->state = NRF_DRV_STATE_INITIALIZED;

    NRF_LOG_INFO("Init");

    err_code = NRF_SUCCESS;
    NRF_LOG_INFO("Function: %s, error code: %s.",
                 (uint32_t)__func__,
                 (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

//-----------------------------------------------------------------------------
//
void nrf_drv_spi_uninit(nrf_drv_spi_t const * const p_instance)
{
    spi_control_block_t * p_cb = &m_cb[p_instance->instance_id];
    ASSERT(p_cb->state != NRF_DRV_STATE_UNINITIALIZED);

    if (p_cb->handler) {
        nrf_drv_common_irq_disable(p_instance->irq);
    }

    NRF_SPIM_Type * p_spim = (NRF_SPIM_Type *)p_instance->p_registers;
    if (p_cb->handler)
    {
        nrf_spim_int_disable(p_spim, 0xFFFFFFFF);
        if (p_cb->transfer_in_progress)
        {
            // Ensure that SPI is not performing any transfer.
            nrf_spim_task_trigger(p_spim, NRF_SPIM_TASK_STOP);
            while (!nrf_spim_event_check(p_spim, NRF_SPIM_EVENT_STOPPED)) {}
            p_cb->transfer_in_progress = false;
        }
    }
    nrf_spim_event_clear(p_spim, NRF_SPIM_EVENT_END);
    nrf_spim_disable(p_spim);
        
    p_cb->state = NRF_DRV_STATE_UNINITIALIZED;
}

//-----------------------------------------------------------------------------
//
ret_code_t nrf_drv_spi_transfer(nrf_drv_spi_t const * const p_instance,
                                uint8_t const * p_tx_buffer,
                                uint8_t         tx_buffer_length,
                                uint8_t       * p_rx_buffer,
                                uint8_t         rx_buffer_length)
{
    nrf_drv_spi_xfer_desc_t xfer_desc;
    xfer_desc.p_tx_buffer = p_tx_buffer;
    xfer_desc.p_rx_buffer = p_rx_buffer;
    xfer_desc.tx_length   = tx_buffer_length;
    xfer_desc.rx_length   = rx_buffer_length;

    NRF_LOG_INFO("Transfer tx_len:%d, rx_len:%d.", tx_buffer_length, rx_buffer_length);
    NRF_LOG_DEBUG("Tx data:");
    NRF_LOG_HEXDUMP_DEBUG((uint8_t *)p_tx_buffer, tx_buffer_length * sizeof(p_tx_buffer[0]));
    return nrf_drv_spi_xfer(p_instance, &xfer_desc, 0);
}

//-----------------------------------------------------------------------------
//
static void finish_transfer(spi_control_block_t * p_cb)
{
    // If Slave Select signal is used, this is the time to deactivate it.
    if (p_cb->ss_pin != NRF_DRV_SPI_PIN_NOT_USED)
    {
        nrf_gpio_pin_set(p_cb->ss_pin);
    }

    // By clearing this flag before calling the handler we allow subsequent
    // transfers to be started directly from the handler function.
    p_cb->transfer_in_progress = false;
    p_cb->evt.type = NRF_DRV_SPI_EVENT_DONE;
    NRF_LOG_INFO("Transfer completed rx_len:%d.", p_cb->evt.data.done.rx_length);
    NRF_LOG_DEBUG("Rx data:");
    NRF_LOG_HEXDUMP_DEBUG((uint8_t *)p_cb->evt.data.done.p_rx_buffer,
                          p_cb->evt.data.done.rx_length * 
                          sizeof(p_cb->evt.data.done.p_rx_buffer[0]));
    p_cb->handler(&p_cb->evt, p_cb->p_context);
}

//-----------------------------------------------------------------------------
//
void spim_int_enable(NRF_SPIM_Type * p_spim, bool enable)
{
    if(!enable) nrf_spim_int_disable(p_spim, END_INT_MASK);
    else        nrf_spim_int_enable (p_spim, END_INT_MASK);
}

//-----------------------------------------------------------------------------
//
void spim_list_enable_handle(NRF_SPIM_Type * p_spim, uint32_t flags)
{
    if (NRF_DRV_SPI_FLAG_TX_POSTINC & flags) {
        nrf_spim_tx_list_enable(p_spim);
    } else {
        nrf_spim_tx_list_disable(p_spim);
    }

    if (NRF_DRV_SPI_FLAG_RX_POSTINC & flags) {
        nrf_spim_rx_list_enable(p_spim);
    } else {
        nrf_spim_rx_list_disable(p_spim);
    }
}

//-----------------------------------------------------------------------------
//
static ret_code_t spim_xfer(NRF_SPIM_Type                * p_spim,
                           spi_control_block_t           * p_cb,
                           nrf_drv_spi_xfer_desc_t const * p_xfer_desc,
                           uint32_t                        flags)
{
    ret_code_t err_code;
    // EasyDMA requires that transfer buffers are placed in Data RAM region;
    // signal error if they are not.
    if ((p_xfer_desc->p_tx_buffer != NULL && !nrf_drv_is_in_RAM(p_xfer_desc->p_tx_buffer)) ||
        (p_xfer_desc->p_rx_buffer != NULL && !nrf_drv_is_in_RAM(p_xfer_desc->p_rx_buffer)))
    {
        p_cb->transfer_in_progress = false;
        err_code = NRF_ERROR_INVALID_ADDR;
        NRF_LOG_WARNING("Function: %s, error code: %s.",
                        (uint32_t)__func__,
                        (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }

    nrf_spim_tx_buffer_set(p_spim, p_xfer_desc->p_tx_buffer, p_xfer_desc->tx_length);
    nrf_spim_rx_buffer_set(p_spim, p_xfer_desc->p_rx_buffer, p_xfer_desc->rx_length);

    nrf_spim_event_clear(p_spim, NRF_SPIM_EVENT_END);

    spim_list_enable_handle(p_spim, flags);

    if (!(flags & NRF_DRV_SPI_FLAG_HOLD_XFER)) {
        nrf_spim_task_trigger(p_spim, NRF_SPIM_TASK_START);
    }

    if (!p_cb->handler) {
        while (!nrf_spim_event_check(p_spim, NRF_SPIM_EVENT_END)){}
        if (p_cb->ss_pin != NRF_DRV_SPI_PIN_NOT_USED) {
            nrf_gpio_pin_set(p_cb->ss_pin);
        }
    }
    else {
        spim_int_enable(p_spim, !(flags & NRF_DRV_SPI_FLAG_NO_XFER_EVT_HANDLER));
    }
    err_code = NRF_SUCCESS;
    NRF_LOG_INFO("Function: %s, error code: %s.",
                 (uint32_t)__func__,
                 (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
    return err_code;
}

//-----------------------------------------------------------------------------

ret_code_t nrf_drv_spi_xfer(nrf_drv_spi_t     const * const p_instance,
                            nrf_drv_spi_xfer_desc_t const * p_xfer_desc,
                            uint32_t                        flags)
{
    spi_control_block_t * p_cb  = &m_cb[p_instance->instance_id];
    ASSERT(p_cb->state != NRF_DRV_STATE_UNINITIALIZED);
    ASSERT(p_xfer_desc->p_tx_buffer != NULL || p_xfer_desc->tx_length == 0);
    ASSERT(p_xfer_desc->p_rx_buffer != NULL || p_xfer_desc->rx_length == 0);

    ret_code_t err_code = NRF_SUCCESS;

    if (p_cb->transfer_in_progress) {
        err_code = NRF_ERROR_BUSY;
        NRF_LOG_WARNING("Function: %s, error code: %s.",
                        (uint32_t)__func__,
                        (uint32_t)NRF_LOG_ERROR_STRING_GET(err_code));
        return err_code;
    }
    else {
        if (p_cb->handler && !(flags & (NRF_DRV_SPI_FLAG_REPEATED_XFER |
                                        NRF_DRV_SPI_FLAG_NO_XFER_EVT_HANDLER)))
        {
            p_cb->transfer_in_progress = true;
        }
    }

    p_cb->evt.data.done = *p_xfer_desc;
    p_cb->tx_done = false;
    p_cb->rx_done = false;
    p_cb->abort   = false;

    if (p_cb->ss_pin != NRF_DRV_SPI_PIN_NOT_USED) {
        nrf_gpio_pin_clear(p_cb->ss_pin);
    }
    return spim_xfer(p_instance->p_registers, p_cb,  p_xfer_desc, flags);
}

//-----------------------------------------------------------------------------

void nrf_drv_spi_abort(nrf_drv_spi_t const * p_instance)
{
    spi_control_block_t * p_cb = &m_cb[p_instance->instance_id];
    ASSERT(p_cb->state != NRF_DRV_STATE_UNINITIALIZED);
    nrf_spim_task_trigger(p_instance->p_registers, NRF_SPIM_TASK_STOP);
    while (!nrf_spim_event_check(p_instance->p_registers, NRF_SPIM_EVENT_STOPPED)) {}
    p_cb->transfer_in_progress = false;
}


//-----------------------------------------------------------------------------

static void irq_handler_spim(NRF_SPIM_Type * p_spim, spi_control_block_t * p_cb)
{

#if NRF_MODULE_ENABLED(SPIM_NRF52_ANOMALY_109_WORKAROUND)
    if ((nrf_spim_int_enable_check(p_spim, NRF_SPIM_INT_STARTED_MASK)) &&
        (nrf_spim_event_check(p_spim, NRF_SPIM_EVENT_STARTED)) )
    {
        /* Handle first, zero-length, auxiliary transmission. */
        nrf_spim_event_clear(p_spim, NRF_SPIM_EVENT_STARTED);
        nrf_spim_event_clear(p_spim, NRF_SPIM_EVENT_END);

        ASSERT(p_spim->TXD.MAXCNT == 0);
        p_spim->TXD.MAXCNT = p_cb->tx_length;

        ASSERT(p_spim->RXD.MAXCNT == 0);
        p_spim->RXD.MAXCNT = p_cb->rx_length;

        /* Disable STARTED interrupt, used only in auxiliary transmission. */
        nrf_spim_int_disable(p_spim, NRF_SPIM_INT_STARTED_MASK);

        /* Start the actual, glitch-free transmission. */
        nrf_spim_task_trigger(p_spim, NRF_SPIM_TASK_START);
        return;
    }
#endif

    if (nrf_spim_event_check(p_spim, NRF_SPIM_EVENT_END))
    {
        nrf_spim_event_clear(p_spim, NRF_SPIM_EVENT_END);
        ASSERT(p_cb->handler);
        NRF_LOG_DEBUG("SPIM: Event: NRF_SPIM_EVENT_END.");
        finish_transfer(p_cb);
    }
}

//-----------------------------------------------------------------------------
//
uint32_t nrf_drv_spi_start_task_get(nrf_drv_spi_t const * p_instance)
{
    NRF_SPIM_Type * p_spim = (NRF_SPIM_Type *)p_instance->p_registers;
    return nrf_spim_task_address_get(p_spim, NRF_SPIM_TASK_START);
}
//
uint32_t nrf_drv_spi_end_event_get(nrf_drv_spi_t const * p_instance)
{
    NRF_SPIM_Type * p_spim = (NRF_SPIM_Type *)p_instance->p_registers;
    return nrf_spim_event_address_get(p_spim, NRF_SPIM_EVENT_END);
}

//-----------------------------------------------------------------------------
//
void SPI0_IRQ_HANDLER(void) {
    spi_control_block_t * p_cb  = &m_cb[SPI0_INSTANCE_INDEX];
    irq_handler_spim(NRF_SPIM0, p_cb);
}
//
void SPI1_IRQ_HANDLER(void) {
    spi_control_block_t * p_cb  = &m_cb[SPI1_INSTANCE_INDEX];
    irq_handler_spim(NRF_SPIM1, p_cb);
}
//
void SPI2_IRQ_HANDLER(void) {
    spi_control_block_t * p_cb  = &m_cb[SPI2_INSTANCE_INDEX];
    irq_handler_spim(NRF_SPIM2, p_cb);
}
