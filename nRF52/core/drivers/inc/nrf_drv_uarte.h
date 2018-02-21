#ifndef _NRF_DRV_UARTE_H_
#define _NRF_DRV_UARTE_H_

#include "nrf_peripherals.h"
#include "nrf_drv_common.h"
#include "nrf_uarte.h"
#include "nrf_gpio.h"

#ifdef __cplusplus
extern "C" {
#endif


//-----------------------------------------------------------------------------
#define NRF_UARTE_PSEL_DISCONNECTED    0xFFFFFFFF
  
//-----------------------------------------------------------------------------
#define UARTE_DEFAULT_CONFIG_HWFC      NRF_UARTE_HWFC_DISABLED
// <> NRF_UARTE_HWFC_DISABLED
// <> NRF_UARTE_HWFC_ENABLED

//-----------------------------------------------------------------------------
#define UARTE_DEFAULT_CONFIG_PARITY             0
// <0=> Excluded 
// <14=> Included 

//-----------------------------------------------------------------------------
#define UARTE_DEFAULT_CONFIG_IRQ_PRIORITY       7
// <i> Priorities 0,2 (nRF51) and 0,1,4,5 (nRF52) are reserved for SoftDevice
// <0=> 0 (highest) 
// <1=> 1 
// <2=> 2 
// <3=> 3 
// <4=> 4 
// <5=> 5 
// <6=> 6 
// <7=> 7 

//-----------------------------------------------------------------------------
#define UARTE_DEFAULT_CONFIG_BAUDRATE    30801920
// <323584=>    1200    baud | NRF_UARTE_BAUDRATE_1200   
// <643072=>    2400    baud | NRF_UARTE_BAUDRATE_2400   
// <1290240=>   4800    baud | NRF_UARTE_BAUDRATE_4800   
// <2576384=>   9600    baud | NRF_UARTE_BAUDRATE_9600   
// <3862528=>   14400   baud | NRF_UARTE_BAUDRATE_14400  
// <5152768=>   19200   baud | NRF_UARTE_BAUDRATE_19200  
// <7716864=>   28800   baud | NRF_UARTE_BAUDRATE_28800  
// <10289152=>  38400   baud | NRF_UARTE_BAUDRATE_38400  
// <15400960=>  57600   baud | NRF_UARTE_BAUDRATE_57600  
// <20615168=>  76800   baud | NRF_UARTE_BAUDRATE_76800  
// <30801920=>  115200  baud | NRF_UARTE_BAUDRATE_115200 
// <61865984=>  230400  baud | NRF_UARTE_BAUDRATE_230400 
// <67108864=>  250000  baud | NRF_UARTE_BAUDRATE_250000 
// <121634816=> 460800  baud | NRF_UARTE_BAUDRATE_460800 
// <251658240=> 921600  baud | NRF_UARTE_BAUDRATE_921600 
// <268435456=> 1000000 baud | NRF_UARTE_BAUDRATE_1000000

/* ============================================================================
 * @brief     ��������������� �������� UART ��� UARTE
 * ------------------------------------------------------------------------- */
typedef nrf_uarte_hwfc_t              nrf_uart_hwfc_t;
typedef nrf_uarte_parity_t            nrf_uart_parity_t;
typedef nrf_uarte_baudrate_t          nrf_uart_baudrate_t;
typedef nrf_uarte_error_mask_t        nrf_uart_error_mask_t;
typedef nrf_uarte_task_t              nrf_uart_task_t;
typedef nrf_uarte_event_t             nrf_uart_event_t;

/* ============================================================================
 * @brief     C�������� ���������� ��������� UART
 * ------------------------------------------------------------------------- */
typedef struct {
    union {
      NRF_UARTE_Type * p_uarte; // ��������� �� ��������� � ���������� UARTE
      void * p_reg;
    } reg;
    uint8_t drv_inst_idx;       // ������ ���������� ��������
} nrf_drv_uarte_t;

// ������ ��� �������� ���������� �������� UARTE ------------------------------
#define NRF_DRV_UART_INSTANCE(id)                      \
{                                                      \
  .reg          = (void *)CONCAT_2(NRF_UARTE, id),     \
  .drv_inst_idx =  CONCAT_3(UART, id, _INSTANCE_INDEX),\
}

// ��������� ����������� ��������� UARTE ---------------------------------------
enum {
  UART0_INSTANCE_INDEX=0,
  UART1_INSTANCE_INDEX,
  UART_INSTANCE_COUNT
};

/* ============================================================================
 * @brief     C�������� ������������ UART
 * ------------------------------------------------------------------------- */
typedef struct
{
    uint32_t            pseltxd;            // ��� ��������            (TXD)
    uint32_t            pselrxd;            // ��� ������              (RXD)
    uint32_t            pselcts;            // ��� ������� �� �������� (CTS)
    uint32_t            pselrts;            // ��� ���������� �������� (RTS)
    void *              p_context;          // �������� ������������ ����������� ����������
    nrf_uart_hwfc_t     hwfc;               // ���������� �������� ������
    nrf_uart_parity_t   parity;             // ��������� ��������
    nrf_uart_baudrate_t baudrate;           // �������� ��������
    uint8_t             interrupt_priority; // ��������� ����������
    bool                use_easy_dma;       // ������������� EasyDMA
} nrf_drv_uarte_config_t;

// ----------------------------------------------------------------------------
#define NRF_DRV_UARTE_DEFAULT_CONFIG                                             \
    {                                                                            \
        .pseltxd            = NRF_UARTE_PSEL_DISCONNECTED,                       \
        .pselrxd            = NRF_UARTE_PSEL_DISCONNECTED,                       \
        .pselcts            = NRF_UARTE_PSEL_DISCONNECTED,                       \
        .pselrts            = NRF_UARTE_PSEL_DISCONNECTED,                       \
        .p_context          = NULL,                                              \
        .hwfc               = (nrf_uart_hwfc_t)UARTE_DEFAULT_CONFIG_HWFC,        \
        .parity             = (nrf_uart_parity_t)UARTE_DEFAULT_CONFIG_PARITY,    \
        .baudrate           = (nrf_uart_baudrate_t)UARTE_DEFAULT_CONFIG_BAUDRATE,\
        .interrupt_priority = UARTE_DEFAULT_CONFIG_IRQ_PRIORITY,                 \
        .use_easy_dma       = true                                               \
    }

    
/* ============================================================================
 * @brief     ��������� �������
 * ----------------------------------------------------------------------------
 * nrf_drv_uarte_xfer_evt_t  - ������-�������� ������
 * nrf_drv_uarte_error_evt_t - ������ UART
 * nrf_drv_uarte_event_t     - ������� UART
 * nrf_drv_uarte_evt_type_t  - ���� �������
 * ------------------------------------------------------------------------- */

typedef enum {
    NRF_DRV_UART_EVT_TX_DONE,            // �������� ���������
    NRF_DRV_UART_EVT_RX_DONE,            // ����� ��������
    NRF_DRV_UART_EVT_ERROR,              // ������ ���������
} nrf_drv_uarte_evt_type_t;

typedef struct {
    uint8_t * p_data;                    // ��������� �� ������ ��� ������-��������
    uint8_t   bytes;                     // ���������� ������������ ����
} nrf_drv_uarte_xfer_evt_t;

typedef struct {
    nrf_drv_uarte_xfer_evt_t rxtx;        // �������� � �������� (���������� ���������� ������)
    uint32_t                error_mask;  // ����� ������ ������, ��������� �������
} nrf_drv_uarte_error_evt_t;

typedef struct {
    nrf_drv_uarte_evt_type_t type;        // ��� �������
    union {
        nrf_drv_uarte_xfer_evt_t  rxtx;   // ������ �������� ��������
        nrf_drv_uarte_error_evt_t error;  // ������ ��� ������� ������
    } data;
} nrf_drv_uarte_event_t;



/* ============================================================================
 * @brief     ����� ��������� ��������� �� ��������� UART
 * ------------------------------------------------------------------------- */
nrf_drv_uarte_t* nrf_drv_uarte_get_instance(uint8_t instance);



/* ============================================================================
 * @brief     ���������� ���������� UART
 * ----------------------------------------------------------------------------
 * @param[in] p_event    ��������� �� ��������� �������
 * @param[in] p_context  �������� ������������� ��� �������������
 * ------------------------------------------------------------------------- */
typedef void (*nrf_uarte_event_handler_t)(nrf_drv_uarte_event_t * p_event, void * p_context);



/* ============================================================================
 * @brief     ������������� � ��������������� UART
 * ----------------------------------------------------------------------------
 * @param[in] p_instance                 ��������� �� ��������� ��������� UART
 * @param[in] p_config                   ��������� �� ��������� ��������� ������������
 * @param[in] event_handler              ���������������� ���������� �������
 * ----------------------------------------------------------------------------
 * @retval    NRF_SUCCESS                ������� �������������
 * @retval    NRF_ERROR_INVALID_STATE    ������� ��� ������������������
 * ------------------------------------------------------------------------- */

ret_code_t nrf_drv_uarte_init(nrf_drv_uarte_t const *        p_instance,
                             nrf_drv_uarte_config_t const * p_config,
                             nrf_uarte_event_handler_t      event_handler);

void nrf_drv_uarte_uninit(nrf_drv_uarte_t const * p_instance);



/* ============================================================================
 * @brief     ������ ��������� ������� ����� � ������� ��� PPI
 * ------------------------------------------------------------------------- */
uint32_t nrf_drv_uarte_task_address_get(nrf_drv_uarte_t const * p_instance,
                                       nrf_uart_task_t task);

uint32_t nrf_drv_uarte_event_address_get(nrf_drv_uarte_t const * p_instance,
                                        nrf_uart_event_t event);



/* ============================================================================
 * @brief     ������� ��� �������� ������
 * ----------------------------------------------------------------------------
 * @retval    NRF_SUCCESS                 ������� �������������
 * @retval    NRF_ERROR_BUSY              ������� � �������� �������� ������
 * @retval    NRF_ERROR_FORBIDDEN         �������� ��������� ������ ����������
 * @retval    NRF_ERROR_INVALID_ADDR      ������ ��������� �� ������
 * ------------------------------------------------------------------------- */
ret_code_t nrf_drv_uarte_tx(nrf_drv_uarte_t const* p_instance,
                           uint8_t const* const p_data, uint8_t length);

bool nrf_drv_uarte_tx_in_progress(nrf_drv_uarte_t const * p_instance);
void nrf_drv_uarte_tx_abort      (nrf_drv_uarte_t const * p_instance);



/* ============================================================================
 * @brief     ������� ��� ������ ������
 * ----------------------------------------------------------------------------
 * @retval    NRF_SUCCESS                 ������� �������������
 * @retval    NRF_ERROR_BUSY              ������� � �������� ������ ������
 * @retval    NRF_ERROR_FORBIDDEN         ����� �������� ������ ����������
 * @retval    NRF_ERROR_INTERNAL          ������ ���������
 * @retval    NRF_ERROR_INVALID_ADDR      ������ ��������� �� ������
 * ------------------------------------------------------------------------- */
ret_code_t nrf_drv_uarte_rx(nrf_drv_uarte_t const* p_instance,
                           uint8_t* p_data, uint8_t length);

bool nrf_drv_uarte_rx_ready  (nrf_drv_uarte_t const * p_instance);
void nrf_drv_uarte_rx_enable (nrf_drv_uarte_t const * p_instance);
void nrf_drv_uarte_rx_disable(nrf_drv_uarte_t const * p_instance);
void nrf_drv_uarte_rx_abort  (nrf_drv_uarte_t const * p_instance);



/* ============================================================================
 * @brief     ������ ��������� ������� ����� � ������� ��� PPI
 * ------------------------------------------------------------------------- */
uint32_t nrf_drv_uarte_errorsrc_get(nrf_drv_uarte_t const * p_instance);



// ----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // _NRF_DRV_UARTE_H_
