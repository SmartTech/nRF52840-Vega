#ifndef _NRF52_UART_H_
#define _NRF52_UART_H_

#include "types.h"
#include "nrf_delay.h"
#include "nrf_drv_uarte.h"
#include "nRF52_FIFO.h"

// Размерности буферов FIFO
#define RX_BUFFER_SIZE     256
#define TX_BUFFER_SIZE     256

#define nRF52_UART0        UART0_INSTANCE_INDEX // 0
#define nRF52_UART1        UART1_INSTANCE_INDEX // 1

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*nRF52_UART_eventCallback)();

typedef struct {
  nRF52_UART_eventCallback rx_callback;
  nRF52_UART_eventCallback tx_callback;
  uint8_t                  rx_attached;
  uint8_t                  tx_attached;
} nRF52_UART_callback_t;
  

// Инициализация и деинициализация --------------------------------------------
void nRF52_UART_init   (uint8_t instance, uint32_t baud, uint8_t pinRX, uint8_t pinTX);
void nRF52_UART_deinit (uint8_t instance);

// Подключение callback функции по приему данных ------------------------------
void nRF52_UART_attachCallback(uint8_t instance, nRF52_UART_eventCallback function);

// Количество данных в приемном буфере ----------------------------------------
size_t nRF52_UART_available (uint8_t instance);

// Получение последовательности байт по указателю -----------------------------
uint8_t nRF52_UART_read     (uint8_t instance, uint8_t* byte, size_t size);

// Передача последовательности байт по указателю ------------------------------
void nRF52_UART_write       (uint8_t instance, uint8_t* data, size_t size);

// Передача строки ------------------------------------------------------------
void nRF52_UART_print       (uint8_t instance, const char* data);

// Получение/передача одного байта --------------------------------------------
void nRF52_UART_get         (uint8_t instance, uint8_t* byte);
void nRF52_UART_put         (uint8_t instance, uint8_t byte);

// Очистка буферов приема и передачи ------------------------------------------
void nRF52_UART_flush       (uint8_t instance);

uint8_t nRF52_UART_wait_tx(uint8_t instance, uint32_t timeout_us);

#ifdef __cplusplus
}
#endif

#endif // _NRF52_UART_H_
