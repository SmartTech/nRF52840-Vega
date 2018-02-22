#ifndef _NRF5x_UART_H_
#define _NRF5x_UART_H_

#define nRF5x_UART_VERSION   1

#include "types.h"
#include "nrf_delay.h"
#include "nrf_drv_uarte.h"
#include "nRF5x_FIFO.h"

// ����������� ������� FIFO
#define RX_BUFFER_SIZE     4 // ������ ���� ������ 4 ������
#define TX_BUFFER_SIZE     4 // ������ ���� ������ 4 ������

#define nRF5x_UART0        UART0_INSTANCE_INDEX // 0
#define nRF5x_UART1        UART1_INSTANCE_INDEX // 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nRF5x_UART_eventCallback)();

typedef struct {
  nRF5x_UART_eventCallback rx_callback;
  nRF5x_UART_eventCallback tx_callback;
  uint8_t                  rx_attached;
  uint8_t                  tx_attached;
} nRF5x_UART_callback_t;

// ������������� � ��������������� --------------------------------------------
uint8_t nRF5x_UART_init   (uint8_t instance, uint32_t baud, uint8_t pinRX, uint8_t pinTX);
uint8_t nRF5x_UART_deinit (uint8_t instance);

// ����������� callback ������� �� ������ ������ ------------------------------
uint8_t nRF5x_UART_attachCallback(uint8_t instance, nRF5x_UART_eventCallback function);

// ���������� ������ � �������� ������ ----------------------------------------
size_t nRF5x_UART_available (uint8_t instance);

// ��������� ������������������ ���� �� ��������� -----------------------------
uint8_t nRF5x_UART_read     (uint8_t instance, uint8_t* byte, size_t size);

// �������� ������������������ ���� �� ��������� ------------------------------
uint8_t nRF5x_UART_write       (uint8_t instance, uint8_t* data, size_t size);

// �������� ������ ------------------------------------------------------------
uint8_t nRF5x_UART_print       (uint8_t instance, const char* data);

// ���������/�������� ������ ����� --------------------------------------------
uint8_t nRF5x_UART_get         (uint8_t instance, uint8_t* byte);
uint8_t nRF5x_UART_put         (uint8_t instance, uint8_t byte);

// ������� ������� ������ � �������� ------------------------------------------
uint8_t nRF5x_UART_flush       (uint8_t instance);

// �������� �������� ����� � ��������� ----------------------------------------
uint8_t nRF5x_UART_wait_tx(uint8_t instance, uint32_t timeout_us);

#ifdef __cplusplus
}
#endif

#endif // _NRF5x_UART_H_
