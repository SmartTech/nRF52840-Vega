#include "nrf_drv_uarte.h"

/*
#define NRF_LOG_MODULE_NAME uart

#if UART_CONFIG_LOG_ENABLED
#define NRF_LOG_LEVEL       UART_CONFIG_LOG_LEVEL
#define NRF_LOG_INFO_COLOR  UART_CONFIG_INFO_COLOR
#define NRF_LOG_DEBUG_COLOR UART_CONFIG_DEBUG_COLOR
#define EVT_TO_STR(event)   (event == NRF_UART_EVENT_ERROR ? "NRF_UART_EVENT_ERROR" : "UNKNOWN EVENT")
#else //UART_CONFIG_LOG_ENABLED
#define EVT_TO_STR(event)   ""
#define NRF_LOG_LEVEL       0
#endif //UART_CONFIG_LOG_ENABLED
#include "nrf_log.h"
NRF_LOG_MODULE_REGISTER();
*/

//-----------------------------------------------------------------------------

#define NRF_LOG_ERROR(...)
#define NRF_LOG_INFO(...)
#define NRF_LOG_INFO(...)
#define NRF_LOG_DEBUG(...)
#define NRF_LOG_HEXDUMP_DEBUG(...)
#define NRF_LOG_WARNING(...)

//-----------------------------------------------------------------------------

#define TX_COUNTER_ABORT_REQ_VALUE 256

//-----------------------------------------------------------------------------

nrf_drv_uarte_t nrf_drv_uart0 = NRF_DRV_UART_INSTANCE(0);
nrf_drv_uarte_t nrf_drv_uart1 = NRF_DRV_UART_INSTANCE(1);

//-----------------------------------------------------------------------------

typedef struct {
    void                   * p_context;
    nrf_uarte_event_handler_t handler;
    uint8_t          const * p_tx_buffer;
    uint8_t                * p_rx_buffer;
    uint8_t                * p_rx_secondary_buffer;
    volatile uint16_t        tx_counter;
    uint8_t                  tx_buffer_length;
    uint8_t                  rx_buffer_length;
    uint8_t                  rx_secondary_buffer_length;
    volatile uint8_t         rx_counter;
    bool                     rx_enabled;
    nrf_drv_state_t          state;
    bool                     use_easy_dma;
} uarte_control_block_t;

//-----------------------------------------------------------------------------

static uarte_control_block_t m_cb[UART_INSTANCE_COUNT];

//-----------------------------------------------------------------------------

nrf_drv_uarte_t* nrf_drv_uarte_get_instance(uint8_t instance)
{
  if(instance==0) return &nrf_drv_uart0;
  return &nrf_drv_uart1;
}

//-----------------------------------------------------------------------------

void apply_config(nrf_drv_uarte_t const * p_instance, nrf_drv_uarte_config_t const * p_config)
{
    if (p_config->pseltxd != NRF_UARTE_PSEL_DISCONNECTED)
    {
        nrf_gpio_pin_set(p_config->pseltxd);
        nrf_gpio_cfg_output(p_config->pseltxd);
    }
    if (p_config->pselrxd != NRF_UARTE_PSEL_DISCONNECTED)
    {
        nrf_gpio_cfg_input(p_config->pselrxd, NRF_GPIO_PIN_NOPULL);
    }

    nrf_uarte_baudrate_set(p_instance->reg.p_uarte, (nrf_uarte_baudrate_t)p_config->baudrate);
    nrf_uarte_configure(p_instance->reg.p_uarte, (nrf_uarte_parity_t)p_config->parity,
                        (nrf_uarte_hwfc_t)p_config->hwfc);
    nrf_uarte_txrx_pins_set(p_instance->reg.p_uarte, p_config->pseltxd, p_config->pselrxd);
    if (p_config->hwfc == NRF_UARTE_HWFC_ENABLED)
    {
        if (p_config->pselcts != NRF_UARTE_PSEL_DISCONNECTED)
        {
            nrf_gpio_cfg_input(p_config->pselcts, NRF_GPIO_PIN_NOPULL);
        }
        if (p_config->pselrts != NRF_UARTE_PSEL_DISCONNECTED)
        {
            nrf_gpio_pin_set(p_config->pselrts);
            nrf_gpio_cfg_output(p_config->pselrts);
        }
        nrf_uarte_hwfc_pins_set(p_instance->reg.p_uarte, p_config->pselrts, p_config->pselcts);
    }
}

//-----------------------------------------------------------------------------

void interrupts_enable(const nrf_drv_uarte_t * p_instance, uint8_t interrupt_priority)
{
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDRX);
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDTX);
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ERROR);
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_RXTO);
    nrf_uarte_int_enable(p_instance->reg.p_uarte, NRF_UARTE_INT_ENDRX_MASK |
                                     NRF_UARTE_INT_ENDTX_MASK |
                                     NRF_UARTE_INT_ERROR_MASK |
                                     NRF_UARTE_INT_RXTO_MASK);
    nrf_drv_common_irq_enable(nrf_drv_get_IRQn((void *)p_instance->reg.p_uarte), interrupt_priority);
}

//-----------------------------------------------------------------------------

void interrupts_disable(const nrf_drv_uarte_t * p_instance)
{
    nrf_uarte_int_disable(p_instance->reg.p_uarte, NRF_UARTE_INT_ENDRX_MASK |
                                      NRF_UARTE_INT_ENDTX_MASK |
                                      NRF_UARTE_INT_ERROR_MASK |
                                      NRF_UARTE_INT_RXTO_MASK);
    nrf_drv_common_irq_disable(nrf_drv_get_IRQn((void *)p_instance->reg.p_uarte));
}

//-----------------------------------------------------------------------------

void pins_to_default(const nrf_drv_uarte_t * p_instance)
{
    /* Reset pins to default states */
    uint32_t txd;
    uint32_t rxd;
    uint32_t rts;
    uint32_t cts;

    txd = nrf_uarte_tx_pin_get(p_instance->reg.p_uarte);
    rxd = nrf_uarte_rx_pin_get(p_instance->reg.p_uarte);
    rts = nrf_uarte_rts_pin_get(p_instance->reg.p_uarte);
    cts = nrf_uarte_cts_pin_get(p_instance->reg.p_uarte);
    nrf_uarte_txrx_pins_disconnect(p_instance->reg.p_uarte);
    nrf_uarte_hwfc_pins_disconnect(p_instance->reg.p_uarte);

    if (txd != NRF_UARTE_PSEL_DISCONNECTED) nrf_gpio_cfg_default(txd);
    if (rxd != NRF_UARTE_PSEL_DISCONNECTED) nrf_gpio_cfg_default(rxd);
    if (cts != NRF_UARTE_PSEL_DISCONNECTED) nrf_gpio_cfg_default(cts);
    if (rts != NRF_UARTE_PSEL_DISCONNECTED) nrf_gpio_cfg_default(rts);
}

//-----------------------------------------------------------------------------

void uart_enable(const nrf_drv_uarte_t * p_instance) {
    nrf_uarte_enable(p_instance->reg.p_uarte);
}

void uart_disable(const nrf_drv_uarte_t * p_instance) {
    nrf_uarte_disable(p_instance->reg.p_uarte);
}

//-----------------------------------------------------------------------------

ret_code_t nrf_drv_uarte_init(const nrf_drv_uarte_t * p_instance, nrf_drv_uarte_config_t const * p_config,
                             nrf_uarte_event_handler_t event_handler)
{
    ASSERT(p_config);
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    ret_code_t err_code = NRF_SUCCESS;

    if (p_cb->state != NRF_DRV_STATE_UNINITIALIZED)
    {
        err_code = NRF_ERROR_INVALID_STATE;
        NRF_LOG_ERROR("Init failed. id:%d in wrong state", nrf_drv_get_IRQn((void *)p_instance->reg.p_reg));
        return err_code;
    }

    p_cb->use_easy_dma = p_config->use_easy_dma;
    
    apply_config(p_instance, p_config);

    p_cb->handler = event_handler;
    p_cb->p_context = p_config->p_context;

    if (p_cb->handler)
    {
        interrupts_enable(p_instance, p_config->interrupt_priority);
    }

    uart_enable(p_instance);
    p_cb->rx_buffer_length = 0;
    p_cb->rx_secondary_buffer_length = 0;
    p_cb->tx_buffer_length = 0;
    p_cb->state = NRF_DRV_STATE_INITIALIZED;
    p_cb->rx_enabled = false;
    return err_code;
}

//-----------------------------------------------------------------------------

void nrf_drv_uarte_uninit(const nrf_drv_uarte_t * p_instance)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    uart_disable(p_instance);

    if (p_cb->handler)
    {
        interrupts_disable(p_instance);
    }

    pins_to_default(p_instance);

    p_cb->state = NRF_DRV_STATE_UNINITIALIZED;
    p_cb->handler = NULL;
    NRF_LOG_INFO("Uninit id: %d.",  nrf_drv_get_IRQn((void *)p_instance->reg.p_reg));
}

//-----------------------------------------------------------------------------

ret_code_t nrf_drv_uart_tx_for_uarte(const nrf_drv_uarte_t * p_instance)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    ret_code_t err_code = NRF_SUCCESS;

    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDTX);
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_tx_buffer_set(p_instance->reg.p_uarte, p_cb->p_tx_buffer, p_cb->tx_buffer_length);
    nrf_uarte_task_trigger(p_instance->reg.p_uarte, NRF_UARTE_TASK_STARTTX);

    if (p_cb->handler == NULL)
    {
        bool endtx;
        bool txstopped;
        do
        {
            endtx     = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDTX);
            txstopped = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_TXSTOPPED);
        }
        while ((!endtx) && (!txstopped));

        if (txstopped)
        {
            err_code = NRF_ERROR_FORBIDDEN;
        }
        p_cb->tx_buffer_length = 0;
    }
    return err_code;
}

//-----------------------------------------------------------------------------

ret_code_t nrf_drv_uarte_tx(const nrf_drv_uarte_t * p_instance, uint8_t const * const p_data, uint8_t length)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    ASSERT(p_cb->state == NRF_DRV_STATE_INITIALIZED);
    ASSERT(length>0);
    ASSERT(p_data);

    ret_code_t err_code;

    // EasyDMA requires that transfer buffers are placed in DataRAM,
    // signal error if the are not.
    if (!nrf_drv_is_in_RAM(p_data))
    {
        err_code = NRF_ERROR_INVALID_ADDR;
       NRF_LOG_ERROR("Id:%d, Easy-DMA buffer not in RAM: %08x",
                                       nrf_drv_get_IRQn((void *)p_instance->reg.p_reg), p_data);
        return err_code;
    }

    if (nrf_drv_uarte_tx_in_progress(p_instance))
    {
        err_code = NRF_ERROR_BUSY;
        NRF_LOG_WARNING("Id:%d busy",nrf_drv_get_IRQn((void *)p_instance->reg.p_reg));
        return err_code;
    }
    p_cb->tx_buffer_length = length;
    p_cb->p_tx_buffer      = p_data;
    p_cb->tx_counter       = 0;

    NRF_LOG_INFO("TX req id:%d length: %d.",
                 nrf_drv_get_IRQn((void *)p_instance->reg.p_reg),
                 p_cb->tx_buffer_length);
    NRF_LOG_DEBUG("Tx data:");
    NRF_LOG_HEXDUMP_DEBUG((uint8_t *)p_cb->p_tx_buffer,
                          p_cb->tx_buffer_length * sizeof(p_cb->p_tx_buffer[0]));

    return nrf_drv_uart_tx_for_uarte(p_instance);
}

//-----------------------------------------------------------------------------

bool nrf_drv_uarte_tx_in_progress(const nrf_drv_uarte_t * p_instance)
{
    return (m_cb[p_instance->drv_inst_idx].tx_buffer_length != 0);
}

//-----------------------------------------------------------------------------

ret_code_t nrf_drv_uart_rx_for_uarte(const nrf_drv_uarte_t * p_instance, uint8_t * p_data, uint8_t length, bool second_buffer)
{
    ret_code_t err_code = NRF_SUCCESS;
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDRX);
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_RXTO);
    nrf_uarte_rx_buffer_set(p_instance->reg.p_uarte, p_data, length);
    if (!second_buffer)
    {
        nrf_uarte_task_trigger(p_instance->reg.p_uarte, NRF_UARTE_TASK_STARTRX);
    }
    else
    {
        nrf_uarte_shorts_enable(p_instance->reg.p_uarte, NRF_UARTE_SHORT_ENDRX_STARTRX);
    }

    if (m_cb[p_instance->drv_inst_idx].handler == NULL)
    {
        bool endrx;
        bool rxto;
        bool error;
        do {
            endrx  = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDRX);
            rxto   = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_RXTO);
            error  = nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ERROR);
        }while ((!endrx) && (!rxto) && (!error));

        m_cb[p_instance->drv_inst_idx].rx_buffer_length = 0;

        if (error)
        {
            err_code = NRF_ERROR_INTERNAL;
        }

        if (rxto)
        {
            err_code = NRF_ERROR_FORBIDDEN;
        }
    }
    else
    {
        nrf_uarte_int_enable(p_instance->reg.p_uarte, NRF_UARTE_INT_ERROR_MASK | NRF_UARTE_INT_ENDRX_MASK);
    }
    return err_code;
}

//-----------------------------------------------------------------------------

ret_code_t nrf_drv_uarte_rx(const nrf_drv_uarte_t * p_instance, uint8_t * p_data, uint8_t length)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];

    ASSERT(m_cb[p_instance->drv_inst_idx].state == NRF_DRV_STATE_INITIALIZED);
    ASSERT(length>0);

    ret_code_t err_code;

    // EasyDMA requires that transfer buffers are placed in DataRAM,
    // signal error if the are not.
    if (!nrf_drv_is_in_RAM(p_data))
    {
        err_code = NRF_ERROR_INVALID_ADDR;
        NRF_LOG_ERROR("Id:%d, Easy-DMA buffer not in RAM: %08x",
                                                   nrf_drv_get_IRQn((void *)p_instance->reg.p_reg), p_data);
        return err_code;
    }

    bool second_buffer = false;

    if (p_cb->handler)
    {
      nrf_uarte_int_disable(p_instance->reg.p_uarte, NRF_UARTE_INT_ERROR_MASK | NRF_UARTE_INT_ENDRX_MASK);
    }
    if (p_cb->rx_buffer_length != 0)
    {
        if (p_cb->rx_secondary_buffer_length != 0)
        {
            if (p_cb->handler)
            {
              nrf_uarte_int_enable(p_instance->reg.p_uarte, NRF_UARTE_INT_ERROR_MASK | NRF_UARTE_INT_ENDRX_MASK);
            }
            err_code = NRF_ERROR_BUSY;
            NRF_LOG_WARNING("RX Id:%d, busy", nrf_drv_get_IRQn((void *)p_instance->reg.p_reg));
            return err_code;
        }
        second_buffer = true;
    }

    if (!second_buffer)
    {
        p_cb->rx_buffer_length = length;
        p_cb->p_rx_buffer      = p_data;
        p_cb->rx_counter       = 0;
        p_cb->rx_secondary_buffer_length = 0;
    }
    else
    {
        p_cb->p_rx_secondary_buffer = p_data;
        p_cb->rx_secondary_buffer_length = length;
    }

    NRF_LOG_INFO("RX Id:%d len:%d", nrf_drv_get_IRQn((void *)p_instance->reg.p_reg), length);

    return nrf_drv_uart_rx_for_uarte(p_instance, p_data, length, second_buffer);
}

//-----------------------------------------------------------------------------

bool nrf_drv_uarte_rx_ready(nrf_drv_uarte_t const * p_instance)
{
   return nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ENDRX);
}

//-----------------------------------------------------------------------------

void nrf_drv_uarte_rx_enable(const nrf_drv_uarte_t * p_instance)
{
    //Easy dma mode does not support enabling receiver without setting up buffer.
    ASSERT(false);
}

//-----------------------------------------------------------------------------

void nrf_drv_uarte_rx_disable(const nrf_drv_uarte_t * p_instance)
{
    //Easy dma mode does not support enabling receiver without setting up buffer.
    ASSERT(false);
}

//-----------------------------------------------------------------------------

uint32_t nrf_drv_uarte_errorsrc_get(const nrf_drv_uarte_t * p_instance)
{
    uint32_t errsrc;
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_ERROR);
    errsrc = nrf_uarte_errorsrc_get_and_clear(p_instance->reg.p_uarte);
    return errsrc;
}

//-----------------------------------------------------------------------------

void rx_done_event(uarte_control_block_t * p_cb, uint8_t bytes, uint8_t * p_data)
{
    nrf_drv_uarte_event_t event;

    event.type             = NRF_DRV_UART_EVT_RX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = p_data;

    p_cb->handler(&event, p_cb->p_context);
}

//-----------------------------------------------------------------------------

void tx_done_event(uarte_control_block_t * p_cb, uint8_t bytes)
{
    nrf_drv_uarte_event_t event;

    event.type             = NRF_DRV_UART_EVT_TX_DONE;
    event.data.rxtx.bytes  = bytes;
    event.data.rxtx.p_data = (uint8_t *)p_cb->p_tx_buffer;

    p_cb->tx_buffer_length = 0;

    NRF_LOG_INFO("TX done len:%d", bytes);
    p_cb->handler(&event, p_cb->p_context);
}

//-----------------------------------------------------------------------------

void nrf_drv_uarte_tx_abort(const nrf_drv_uarte_t * p_instance)
{
    uarte_control_block_t * p_cb = &m_cb[p_instance->drv_inst_idx];
    nrf_uarte_event_clear(p_instance->reg.p_uarte, NRF_UARTE_EVENT_TXSTOPPED);
    nrf_uarte_task_trigger(p_instance->reg.p_uarte, NRF_UARTE_TASK_STOPTX);
    if (p_cb->handler == NULL)
    {
        while (!nrf_uarte_event_check(p_instance->reg.p_uarte, NRF_UARTE_EVENT_TXSTOPPED));
    }
    NRF_LOG_INFO("TX abort Id:%d", nrf_drv_get_IRQn((void *)p_instance->reg.p_reg));
}

//-----------------------------------------------------------------------------

void nrf_drv_uarte_rx_abort(const nrf_drv_uarte_t * p_instance)
{
    nrf_uarte_task_trigger(p_instance->reg.p_uarte, NRF_UARTE_TASK_STOPRX);
    NRF_LOG_INFO("RX abort Id:%d", nrf_drv_get_IRQn((void *)p_instance->reg.p_reg));
}

//-----------------------------------------------------------------------------

void uarte_irq_handler(NRF_UARTE_Type * p_uarte, uarte_control_block_t * p_cb)
{
    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ERROR))
    {
        nrf_drv_uarte_event_t event;

        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ERROR);

        event.type                   = NRF_DRV_UART_EVT_ERROR;
        event.data.error.error_mask  = nrf_uarte_errorsrc_get_and_clear(p_uarte);
        event.data.error.rxtx.bytes  = nrf_uarte_rx_amount_get(p_uarte);
        event.data.error.rxtx.p_data = p_cb->p_rx_buffer;

        //abort transfer
        p_cb->rx_buffer_length = 0;
        p_cb->rx_secondary_buffer_length = 0;

        p_cb->handler(&event, p_cb->p_context);
    }
    else if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ENDRX))
    {
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ENDRX);
        uint8_t amount = nrf_uarte_rx_amount_get(p_uarte);
        // If the transfer was stopped before completion, amount of transfered bytes
        // will not be equal to the buffer length. Interrupted trunsfer is ignored.
        if (amount == p_cb->rx_buffer_length) {
            if (p_cb->rx_secondary_buffer_length) {
                uint8_t * p_data = p_cb->p_rx_buffer;
                nrf_uarte_shorts_disable(p_uarte, NRF_UARTE_SHORT_ENDRX_STARTRX);
                p_cb->rx_buffer_length = p_cb->rx_secondary_buffer_length;
                p_cb->p_rx_buffer = p_cb->p_rx_secondary_buffer;
                p_cb->rx_secondary_buffer_length = 0;
                rx_done_event(p_cb, amount, p_data);
            }
            else {
                p_cb->rx_buffer_length = 0;
                rx_done_event(p_cb, amount, p_cb->p_rx_buffer);
            }
        }
    }

    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_RXTO)) {
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_RXTO);
        if (p_cb->rx_buffer_length) {
            p_cb->rx_buffer_length = 0;
            rx_done_event(p_cb, nrf_uarte_rx_amount_get(p_uarte), p_cb->p_rx_buffer);
        }
    }

    if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ENDTX)) {
        nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ENDTX);
        if (p_cb->tx_buffer_length) {
            tx_done_event(p_cb, nrf_uarte_tx_amount_get(p_uarte));
        }
    }
}

//-----------------------------------------------------------------------------

void UART0_IRQHandler(void) {
    uarte_irq_handler(NRF_UARTE0, &m_cb[UART0_INSTANCE_INDEX]);
}

void UARTE1_IRQHandler(void) {
    uarte_irq_handler(NRF_UARTE1, &m_cb[UART1_INSTANCE_INDEX]);
}
