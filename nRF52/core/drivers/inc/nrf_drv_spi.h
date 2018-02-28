#ifndef _NRF_DRV_SPI_H_
#define _NRF_DRV_SPI_H_

#include "nrf_drv_common.h"

#include "nrf_spi.h"
#include "nrf_spim.h"
#include "nrf_spis.h"

#include "nrf_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif
  

// <0=> NRF_GPIO_PIN_NOPULL 
// <1=> NRF_GPIO_PIN_PULLDOWN 
// <3=> NRF_GPIO_PIN_PULLUP
#define NRF_SPI_DRV_MISO_PULL_CFG       NRF_GPIO_PIN_PULLDOWN
  
#define SPI_N_USE_EASY_DMA(_id)         true
  
#define NRF_DRV_SPI_PERIPHERAL(id)      (void *)CONCAT_2(NRF_SPIM, id)

#define NRF_DRV_SPI_PIN_NOT_USED  0xFF

// Ќумераци€ экземпл€ров переферии SPI ----------------------------------------
enum {
  SPI0_INSTANCE_INDEX=0,
  SPI1_INSTANCE_INDEX,
  SPI2_INSTANCE_INDEX,
  SPI3_INSTANCE_INDEX,
  SPI_INSTANCE_COUNT
};

//  оличество возможных экземпл€ров дл€ каждого режима ------------------------
#define SPI_MASTER_COUNT    SPI_INSTANCE_COUNT     // все экземпл€ры могут быть мастером
#define SPI_SLAVE_COUNT     SPI_INSTANCE_COUNT - 1 // первые 3 экземпл€ра могут быть слэйвом

// ќбработчики прерываний -----------------------------------------------------
#define SPI0_IRQ            SPIM0_SPIS0_TWIM0_TWIS0_SPI0_TWI0_IRQn
#define SPI1_IRQ            SPIM1_SPIS1_TWIM1_TWIS1_SPI1_TWI1_IRQn
#define SPI2_IRQ            SPIM2_SPIS2_SPI2_IRQn
#define SPI3_IRQ            SPIM3_IRQn

#define SPI0_IRQ_HANDLER    SPI0_TWI0_IRQHandler


/* ============================================================================
 * @brief —труктура экземпл€ра драйвера SPI
 * ------------------------------------------------------------------------- */
typedef struct {
    void *    p_registers;  // Pointer to the structure with SPI/SPIM peripheral instance registers.
    IRQn_Type irq;          // SPI/SPIM peripheral instance IRQ number.
    uint8_t   instance_id;  // Driver instance index.
    bool      use_easy_dma; // True if the peripheral with EasyDMA (SPIM) shall be used.
} nrf_drv_spi_t;



/* ============================================================================
 * @brief Macro for creating an SPI master driver instance.
 * ------------------------------------------------------------------------- */
#define NRF_DRV_SPI_INSTANCE(id)                        \
{                                                       \
    .p_registers  = NRF_DRV_SPI_PERIPHERAL(id),         \
    .irq          = CONCAT_3(SPI, id, _IRQ),            \
    .instance_id  = CONCAT_3(SPI, id, _INSTANCE_INDEX), \
    .use_easy_dma = SPI_N_USE_EASY_DMA(id)              \
}



/* ============================================================================
 * @brief SPI data rates.
 * ------------------------------------------------------------------------- */
typedef enum {
    NRF_DRV_SPI_FREQ_125K = NRF_SPI_FREQ_125K, // 125 kbps.
    NRF_DRV_SPI_FREQ_250K = NRF_SPI_FREQ_250K, // 250 kbps.
    NRF_DRV_SPI_FREQ_500K = NRF_SPI_FREQ_500K, // 500 kbps.
    NRF_DRV_SPI_FREQ_1M   = NRF_SPI_FREQ_1M,   // 1 Mbps.
    NRF_DRV_SPI_FREQ_2M   = NRF_SPI_FREQ_2M,   // 2 Mbps.
    NRF_DRV_SPI_FREQ_4M   = NRF_SPI_FREQ_4M,   // 4 Mbps.
    NRF_DRV_SPI_FREQ_8M   = NRF_SPI_FREQ_8M    // 8 Mbps.
} nrf_drv_spi_frequency_t;



/* ============================================================================
 * @brief SPI modes.
 * ------------------------------------------------------------------------- */
typedef enum {
    NRF_DRV_SPI_MODE_0 = NRF_SPI_MODE_0, // SCK active high, sample on leading edge of clock.
    NRF_DRV_SPI_MODE_1 = NRF_SPI_MODE_1, // SCK active high, sample on trailing edge of clock.
    NRF_DRV_SPI_MODE_2 = NRF_SPI_MODE_2, // SCK active low, sample on leading edge of clock.
    NRF_DRV_SPI_MODE_3 = NRF_SPI_MODE_3  // SCK active low, sample on trailing edge of clock.
} nrf_drv_spi_mode_t;



/* ============================================================================
 * @brief SPI bit orders
 * ------------------------------------------------------------------------- */
typedef enum {
    NRF_DRV_SPI_BIT_ORDER_MSB_FIRST = NRF_SPI_BIT_ORDER_MSB_FIRST, // Most significant bit shifted out first.
    NRF_DRV_SPI_BIT_ORDER_LSB_FIRST = NRF_SPI_BIT_ORDER_LSB_FIRST  // Least significant bit shifted out first.
} nrf_drv_spi_bit_order_t;



/* ============================================================================
 * @brief —труктура конфигурации SPI
 * ------------------------------------------------------------------------- */
typedef struct {
    uint8_t                 sck_pin;      // SCK pin number.
    uint8_t                 mosi_pin;     // MOSI pin number (optional).
    uint8_t                 miso_pin;     // MISO pin number (optional).
    uint8_t                 ss_pin;       // Slave Select pin number (optional)
    uint8_t                 irq_priority; // Interrupt priority.
    uint8_t                 orc;          // Over-run character.
    uint8_t                 def;          // Character clocked out in case of an ignored transaction.
    nrf_gpio_pin_drive_t    miso_drive;   // MISO pin drive configuration.
    nrf_gpio_pin_pull_t     csn_pullup;   // CSN pin pull-up configuration.
    nrf_drv_spi_frequency_t frequency;    // SPI frequency.
    nrf_drv_spi_mode_t      mode;         // SPI mode.
    nrf_drv_spi_bit_order_t bit_order;    // SPI bit order.
} nrf_drv_spi_config_t;



/* ============================================================================
 * @brief SPI master instance default configuration.
 * ------------------------------------------------------------------------- */
#define NRF_DRV_SPI_DEFAULT_CONFIG                           \
{                                                            \
    .sck_pin      = NRF_DRV_SPI_PIN_NOT_USED,                \
    .mosi_pin     = NRF_DRV_SPI_PIN_NOT_USED,                \
    .miso_pin     = NRF_DRV_SPI_PIN_NOT_USED,                \
    .ss_pin       = NRF_DRV_SPI_PIN_NOT_USED,                \
    .orc          = 0xFF,                                    \
    .def          = SPIS_DEFAULT_DEF,                        \
    .mode         = NRF_DRV_SPI_MODE_0,                      \
    .miso_drive   = NRF_DRV_SPIS_DEFAULT_MISO_DRIVE,         \
    .frequency    = NRF_DRV_SPI_FREQ_4M,                     \
    .bit_order    = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST,         \
    .irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,         \
}



#define NRF_DRV_SPI_FLAG_TX_POSTINC          (1UL << 0) // TX buffer address incremented after transfer
#define NRF_DRV_SPI_FLAG_RX_POSTINC          (1UL << 1) // RX buffer address incremented after transfer
#define NRF_DRV_SPI_FLAG_NO_XFER_EVT_HANDLER (1UL << 2) // Interrupt after each transfer is suppressed, and the event handler is not called
#define NRF_DRV_SPI_FLAG_HOLD_XFER           (1UL << 3) // Set up the transfer but do not start it
#define NRF_DRV_SPI_FLAG_REPEATED_XFER       (1UL << 4) // Flag indicating that the transfer will be executed multiple times



/* ============================================================================
 * @brief Single transfer descriptor structure.
 * ------------------------------------------------------------------------- */
typedef struct {
    uint8_t const * p_tx_buffer; // Pointer to TX buffer.
    uint8_t         tx_length;   // TX buffer length.
    uint8_t       * p_rx_buffer; // Pointer to RX buffer.
    uint8_t         rx_length;   // RX buffer length.
} nrf_drv_spi_xfer_desc_t;



/* ============================================================================
 * @brief Macro for setting up single transfer descriptor.
 * ----------------------------------------------------------------------------
 * This macro is for internal use only.
 * ------------------------------------------------------------------------- */
#define NRF_DRV_SPI_SINGLE_XFER(p_tx, tx_len, p_rx, rx_len)  \
    {                                                        \
    .p_tx_buffer = (uint8_t const *)(p_tx),                  \
    .tx_length   = (tx_len),                                 \
    .p_rx_buffer = (p_rx),                                   \
    .rx_length   = (rx_len),                                 \
    }

    
    
/* ============================================================================
 * @brief Macro for setting duplex TX RX transfer.
 * ------------------------------------------------------------------------- */
#define NRF_DRV_SPI_XFER_TRX(p_tx_buf, tx_length, p_rx_buf, rx_length)        \
        NRF_DRV_SPI_SINGLE_XFER(p_tx_buf, tx_length, p_rx_buf, rx_length)

/* ============================================================================
 * @brief Macro for setting TX transfer.
 * ------------------------------------------------------------------------- */
#define NRF_DRV_SPI_XFER_TX(p_buf, length) \
        NRF_DRV_SPI_SINGLE_XFER(p_buf, length, NULL, 0)

          
          
/* ============================================================================
 * @brief Macro for setting RX transfer.
 * ------------------------------------------------------------------------- */
#define NRF_DRV_SPI_XFER_RX(p_buf, length) \
          NRF_DRV_SPI_SINGLE_XFER(NULL, 0, p_buf, length)

          
          
/* ============================================================================
 * @brief SPI master driver event types, passed to the handler routine provided
 *        during initialization.
 * ------------------------------------------------------------------------- */
typedef enum {
    NRF_DRV_SPI_EVENT_DONE, ///< Transfer done.
} nrf_drv_spi_evt_type_t;

typedef struct {
    nrf_drv_spi_evt_type_t  type;      ///< Event type.
    union {
        nrf_drv_spi_xfer_desc_t done;  ///< Event data for DONE event.
    } data;
} nrf_drv_spi_evt_t;



/* ============================================================================
 * @brief     метод получени€ указател€ на экземпл€р SPI
 * ------------------------------------------------------------------------- */
nrf_drv_spi_t* nrf_drv_spi_get_instance(uint8_t instance);



/* ============================================================================
 * @brief “ип обработчика событий
 * ------------------------------------------------------------------------- */
typedef void (* nrf_drv_spi_evt_handler_t)(nrf_drv_spi_evt_t const * p_event,
                                           void                    * p_context);



/* ============================================================================
 * @brief ‘ункци€ инициализаци€ экземпл€ра SPI master
 * ----------------------------------------------------------------------------
 * @note MISO pin has pull down enabled.
 * ----------------------------------------------------------------------------
 * @param[in] p_instance           указатель на экземпл€р структуры SPI
 * @param[in] p_config             указатель на экземпл€р структуры конфигурации
 * ----------------------------------------------------------------------------
 * @param     handler              обработчик событий
 * @param     p_context            контекст передаваемый обработчику
 * ----------------------------------------------------------------------------
 * @retval NRF_SUCCESS             удачна€ инициализаци€
 * @retval NRF_ERROR_INVALID_STATE драйвер уже проинициализирован
 * @retval NRF_ERROR_BUSY          уже используетс€
 * ------------------------------------------------------------------------- */
ret_code_t nrf_drv_spi_init(nrf_drv_spi_t const * const p_instance,
                            nrf_drv_spi_config_t const * p_config,
                            nrf_drv_spi_evt_handler_t handler,
                            void *    p_context);



/* ============================================================================
 * @brief ‘ункци€ деинициализаци€ экземпл€ра SPI master
 * ----------------------------------------------------------------------------
 * @note при этом конфигураци€ пинов сохран€етс€
 * ----------------------------------------------------------------------------
 * @param[in] p_instance           указатель на экземпл€р структуры SPI
 * ------------------------------------------------------------------------- */
void nrf_drv_spi_uninit(nrf_drv_spi_t const * const p_instance);



/* ============================================================================
 * @brief ‘ункци€ запуска передачи данных
 * ----------------------------------------------------------------------------
 * @note ѕерефери€ использует EasyDMA, необходим передающий буфер
 * ----------------------------------------------------------------------------
 * @param[in] p_instance           указатель на экземпл€р структуры SPI
 * @param[in] p_tx_buffer          указатель на передающий буфер
 * @param     tx_buffer_length     размерность передающего буфера
 * @param[in] p_rx_buffer          указатель на принимающий буфера
 * @param                          размерность принимающего буфера
 * ----------------------------------------------------------------------------
 * @retval NRF_SUCCESS             удачна€ операци€
 * @retval NRF_ERROR_BUSY          если предыдуща€ передача не закончена
 * @retval NRF_ERROR_INVALID_ADDR  если буфер не положен в регион пам€ти
 * ------------------------------------------------------------------------- */
ret_code_t nrf_drv_spi_transfer(nrf_drv_spi_t const * const p_instance,
                                uint8_t const * p_tx_buffer,
                                uint8_t         tx_buffer_length,
                                uint8_t       * p_rx_buffer,
                                uint8_t         rx_buffer_length);



/* ============================================================================
 * @brief ‘ункци€ запуска передачи данных с дополнительными флагами
 * ----------------------------------------------------------------------------
 * @param p_instance               указатель на экземпл€р структуры SPI
 * @param p_xfer_desc              ”казатель на дескриптор передачи
 * @param flags                    ќпции передачи(0 - по умолчанию)
 * ----------------------------------------------------------------------------
 * @retval NRF_SUCCESS             удачна€ операци€
 * @retval NRF_ERROR_BUSY          драйвер не готов к новой передаче
 * @retval NRF_ERROR_NOT_SUPPORTED указанные параметры не поддерживаютс€
 * @retval NRF_ERROR_INVALID_ADDR  если буфер не положен в регион пам€ти
 * ------------------------------------------------------------------------- */
ret_code_t nrf_drv_spi_xfer(nrf_drv_spi_t     const * const p_instance,
                            nrf_drv_spi_xfer_desc_t const * p_xfer_desc,
                            uint32_t                        flags);



/* ============================================================================
 * @brief ‘ункци€ дл€ получени€ адреса задачи запуска SPIM дл€ PPI
 * ----------------------------------------------------------------------------
 * @param[in]  p_instance          указатель на экземпл€р структуры SPI
 * ----------------------------------------------------------------------------
 * @return                         адрес запуска задачи
 * ------------------------------------------------------------------------- */
uint32_t nrf_drv_spi_start_task_get(nrf_drv_spi_t const * p_instance);



/* ============================================================================
 * @brief ‘ункци€ дл€ получени€ адреса событи€ окончани€ передачи SPIM
 * ----------------------------------------------------------------------------
 * @param[in]  p_instance          указатель на экземпл€р структуры SPI
 * ----------------------------------------------------------------------------
 * @return                         адрес событи€ завершени€ передачи
 * ------------------------------------------------------------------------- */
uint32_t nrf_drv_spi_end_event_get(nrf_drv_spi_t const * p_instance);



/* ============================================================================
 * @brief ‘ункци€ отмены текущей передачи
 * ----------------------------------------------------------------------------
 * @param[in]  p_instance          указатель на экземпл€р структуры SPI
 * ------------------------------------------------------------------------- */
void nrf_drv_spi_abort(nrf_drv_spi_t const * p_instance);



#ifdef __cplusplus
}
#endif

#endif // _NRF_DRV_SPI_H_
