#include "nRF5x_UART.h"


//-----------------------------------------------------------------------------
// Флгаи инициализации экземпляров UART
uint8_t nRF5x_UART_inited_flag[UART_INSTANCE_COUNT] = {false, false};

//-----------------------------------------------------------------------------
// Экземпляры структур FIFO
nRF5x_FIFO_t nRF5x_UART_rx_fifo[UART_INSTANCE_COUNT];
nRF5x_FIFO_t nRF5x_UART_tx_fifo[UART_INSTANCE_COUNT];

//-----------------------------------------------------------------------------
// Буферы приема и передачи
uint8_t nRF5x_UART_rx_buffer[UART_INSTANCE_COUNT][RX_BUFFER_SIZE];
uint8_t nRF5x_UART_tx_buffer[UART_INSTANCE_COUNT][TX_BUFFER_SIZE];

//-----------------------------------------------------------------------------
// Флаги переполнения приемных буферов
uint8_t nRF5x_UART_rx_ovf[UART_INSTANCE_COUNT];

//-----------------------------------------------------------------------------
nRF5x_UART_callback_t nRF5x_UART_callback[UART_INSTANCE_COUNT];

//-----------------------------------------------------------------------------
// Обработчик приема данных по UART
void nRF5x_UART_handler(uint8_t instance, nrf_drv_uarte_event_t * p_event)
{
  uint32_t err_code;
  
  nrf_drv_uarte_t uart = *nrf_drv_uarte_get_instance(instance);
  
  switch (p_event->type) {
    
    // Передача байта завершена -----------------------------------------------
    case NRF_DRV_UART_EVT_RX_DONE:
        // Записываем принятый байт в буфер FIFO
        err_code = nRF5x_FIFO_put(&nRF5x_UART_rx_fifo[instance], p_event->data.rxtx.p_data[0]);
        // Если запись в FIFO не удалась
        if (err_code != NRF_SUCCESS) {
            //app_uart_event.evt_type          = APP_UART_FIFO_ERROR;
            //app_uart_event.data.error_code   = err_code;
            //m_event_handler(&app_uart_event);
        }
        // Иначе если в FIFO есть данные
        else if (nRF5x_FIFO_length(&nRF5x_UART_rx_fifo[instance]) != 0)
        {
            // Вызываем callback функцию (если назначена)
            if(nRF5x_UART_callback[instance].rx_attached) {
              nRF5x_UART_callback[instance].rx_callback();
            }
            //app_uart_event.evt_type = APP_UART_DATA_READY;
            //m_event_handler(&app_uart_event);
        }
        //---------------------------------------------------------------------
        // Если буфер не переполнен, то запускаем прием следующего байта
        if (nRF5x_FIFO_length(&nRF5x_UART_rx_fifo[instance]) <= nRF5x_UART_rx_fifo[instance].buf_size_mask) {
            (void)nrf_drv_uarte_rx(&uart, nRF5x_UART_rx_buffer[instance], 1);
        }
        // иначе выставляем флаг переполнения
        else {
            nRF5x_UART_rx_ovf[instance] = true; // Переполнение приемного буфера FIFO.
        }
        break;
        
    // Прием байта завершен ---------------------------------------------------
    case NRF_DRV_UART_EVT_TX_DONE:
        // Берем следующий байт из FIFO, если есть
        if (nRF5x_FIFO_get(&nRF5x_UART_tx_fifo[instance], nRF5x_UART_tx_buffer[instance]) == NRF_SUCCESS) {
            // И отправляем на передачу в UART
            nrf_drv_uarte_tx(&uart, nRF5x_UART_tx_buffer[instance], 1);
        }
        // иначе был передан последний байт из FIFO
        else {
            //app_uart_event.evt_type = APP_UART_TX_EMPTY;
            //m_event_handler(&app_uart_event);
        }
        break;
    
    default: break;
  }
}

//-----------------------------------------------------------------------------
// Обработчик приема данных экземпляра UARTE0
void nRF5x_UART_0_handler(nrf_drv_uarte_event_t* p_event, void* p_context)
{
  nRF5x_UART_handler(nRF5x_UART0, p_event);
}

//-----------------------------------------------------------------------------
// Обработчик приема данных экземпляра UARTE1
void nRF5x_UART_1_handler(nrf_drv_uarte_event_t* p_event, void* p_context)
{
  nRF5x_UART_handler(nRF5x_UART1, p_event);
}

//-----------------------------------------------------------------------------
// Инициализация порта UART
uint8_t nRF5x_UART_init(uint8_t instance, uint32_t baud, uint8_t pinRX, uint8_t pinTX)
{
  ret_code_t status = NRF_SUCCESS;
  
  // Экземпляр UART не может быть больше чем имеется в МК
  if(instance >= UART_INSTANCE_COUNT) return false;
  
  // Объявление структуры конфигурации с дефолтными параметрами
  nrf_drv_uarte_config_t uartConfig = NRF_DRV_UARTE_DEFAULT_CONFIG;
  
  // Получение структуры экземпляра UART по номеру
  nrf_drv_uarte_t uart = *nrf_drv_uarte_get_instance(instance);
  
  nRF5x_UART_callback[instance].rx_attached = false;
  nRF5x_UART_callback[instance].tx_attached = false;
  
  // Конфигурация пинов
  uartConfig.pselrxd = pinRX;
  uartConfig.pseltxd = pinTX;
  
  // Конфигурация скорости
  switch(baud) {
    case 1200    : uartConfig.baudrate = NRF_UARTE_BAUDRATE_1200;       break;
    case 2400    : uartConfig.baudrate = NRF_UARTE_BAUDRATE_2400;       break;
    case 4800    : uartConfig.baudrate = NRF_UARTE_BAUDRATE_4800;       break;
    case 9600    : uartConfig.baudrate = NRF_UARTE_BAUDRATE_9600;       break;
    case 14400   : uartConfig.baudrate = NRF_UARTE_BAUDRATE_14400;      break;
    case 19200   : uartConfig.baudrate = NRF_UARTE_BAUDRATE_19200;      break;
    case 28800   : uartConfig.baudrate = NRF_UARTE_BAUDRATE_28800;      break;
    case 38400   : uartConfig.baudrate = NRF_UARTE_BAUDRATE_38400;      break;
    case 57600   : uartConfig.baudrate = NRF_UARTE_BAUDRATE_57600;      break;
    case 76800   : uartConfig.baudrate = NRF_UARTE_BAUDRATE_76800;      break;
    case 115200  : uartConfig.baudrate = NRF_UARTE_BAUDRATE_115200;     break;
    case 230400  : uartConfig.baudrate = NRF_UARTE_BAUDRATE_230400;     break;
    case 250000  : uartConfig.baudrate = NRF_UARTE_BAUDRATE_250000;     break;
    case 460800  : uartConfig.baudrate = NRF_UARTE_BAUDRATE_460800;     break;
    case 921600  : uartConfig.baudrate = NRF_UARTE_BAUDRATE_921600;     break;
    case 1000000 : uartConfig.baudrate = NRF_UARTE_BAUDRATE_1000000;    break;
    default: break;
  }
    
  // Инициализируем буферы приема и передачи
  status = nRF5x_FIFO_init(&nRF5x_UART_rx_fifo[instance], nRF5x_UART_rx_buffer[instance], RX_BUFFER_SIZE);
  if(status != NRF_SUCCESS) return false;
  
  status = nRF5x_FIFO_init(&nRF5x_UART_tx_fifo[instance], nRF5x_UART_tx_buffer[instance], TX_BUFFER_SIZE);
  if(status != NRF_SUCCESS) return false;
  
  // Инициализируем экземпляр UART
  if(!instance) status = nrf_drv_uarte_init(&uart, &uartConfig, nRF5x_UART_0_handler);
  else          status = nrf_drv_uarte_init(&uart, &uartConfig, nRF5x_UART_1_handler);
  if(status != NRF_SUCCESS) return false;
  
  // Если пин RX назначен - активируем прием
  if(uartConfig.pselrxd != 255) {
      if(!uartConfig.use_easy_dma) {
          nrf_drv_uarte_rx_enable(&uart);
      }
      status = nrf_drv_uarte_rx(&uart, nRF5x_UART_rx_buffer[instance], 1);
  }
  if(status != NRF_SUCCESS) return false;
    
  // Устанавливаем флаг инициализации
  nRF5x_UART_inited_flag[instance] = 1;
  
  return true;
}

//-----------------------------------------------------------------------------
// Деинициализация порта UART
uint8_t nRF5x_UART_deinit(uint8_t instance)
{
  if(instance >= UART_INSTANCE_COUNT) return false;
  if(nRF5x_UART_inited_flag[instance]) {
    nrf_drv_uarte_t uart = *nrf_drv_uarte_get_instance(instance);
    nrf_drv_uarte_uninit(&uart);
    nRF5x_UART_inited_flag[instance] = 0;
  }
  return !nRF5x_UART_inited_flag[instance];
}

//-----------------------------------------------------------------------------
// Подключение callback функции по приему данных
uint8_t nRF5x_UART_attachCallback(uint8_t instance, nRF5x_UART_eventCallback function)
{
  if(instance >= UART_INSTANCE_COUNT) return false;
  nRF5x_UART_callback[instance].rx_callback = function;
  nRF5x_UART_callback[instance].rx_attached = true;
  return true;
}

//-----------------------------------------------------------------------------
// Количество данных в приемном буфере
size_t nRF5x_UART_available(uint8_t instance)
{
  if(instance >= UART_INSTANCE_COUNT) return false;
  return nRF5x_FIFO_length(&nRF5x_UART_rx_fifo[instance]);
}

//-----------------------------------------------------------------------------
// Получение последовательности байт по указателю
uint8_t nRF5x_UART_read(uint8_t instance, uint8_t* byte, size_t size)
{
  if(instance >= UART_INSTANCE_COUNT) return false;
  for(uint32_t i=0; i<size; i++) nRF5x_UART_get(instance, &byte[i]);
  return true;
}

//-----------------------------------------------------------------------------
// Передача последовательности байт по указателю
uint8_t nRF5x_UART_write(uint8_t instance, uint8_t* data, size_t size)
{
  if(instance >= UART_INSTANCE_COUNT) return false;
  if(!nRF5x_UART_inited_flag[instance]) return false;
  for(uint32_t i=0; i<size; i++) {
    if(!nRF5x_UART_put(instance, data[i])) return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
// Передача строки
uint8_t nRF5x_UART_print(uint8_t instance, const char* data)
{
  return nRF5x_UART_write(instance, (uint8_t*)data, strlen(data));
}

//-----------------------------------------------------------------------------
// Получение одного байта
uint8_t nRF5x_UART_get(uint8_t instance, uint8_t* byte)
{
  ret_code_t status;
  if(instance >= UART_INSTANCE_COUNT) return false;
  nrf_drv_uarte_t uart = *nrf_drv_uarte_get_instance(instance);
  uint8_t rx_ovf = nRF5x_UART_rx_ovf[instance];
  status = nRF5x_FIFO_get(&nRF5x_UART_rx_fifo[instance], byte);
  if(status != NRF_SUCCESS) return false;
  if (rx_ovf) {
      nRF5x_UART_rx_ovf[instance] = false;
      status = nrf_drv_uarte_rx(&uart, nRF5x_UART_rx_buffer[instance], 1);
  }
  return (status == NRF_SUCCESS);
}

//-----------------------------------------------------------------------------
// Передача одного байта
uint8_t nRF5x_UART_put(uint8_t instance, uint8_t byte)
{
  if(instance >= UART_INSTANCE_COUNT) return false;
  nrf_drv_uarte_t uart = *nrf_drv_uarte_get_instance(instance);
  
  ret_code_t status;
  
  if(nRF5x_FIFO_length(&nRF5x_UART_tx_fifo[instance]) <= nRF5x_UART_tx_fifo[instance].buf_size_mask)
  {
    status = nRF5x_FIFO_put(&nRF5x_UART_tx_fifo[instance], byte);
  }
  else
  {
    if(nRF5x_UART_wait_tx(instance, 1000)) status = NRF_SUCCESS;
  }
  
  if (status == NRF_SUCCESS) {
      if (!nrf_drv_uarte_tx_in_progress(&uart)) {
          status = nRF5x_FIFO_get(&nRF5x_UART_tx_fifo[instance], nRF5x_UART_tx_buffer[instance]);
          if(status == NRF_SUCCESS) {
              status = nrf_drv_uarte_tx(&uart, nRF5x_UART_tx_buffer[instance], 1);
          }
      }
  }
  return (status == NRF_SUCCESS);
}

//-----------------------------------------------------------------------------
// Ожидание передачи байта
uint8_t nRF5x_UART_wait_tx(uint8_t instance, uint32_t timeout_us) {
  nrf_drv_uarte_t uart = *nrf_drv_uarte_get_instance(instance);
  while(nrf_drv_uarte_tx_in_progress(&uart)) {
    nrf_delay_us(1);
    if(!timeout_us) return false;
    timeout_us--;
  }
  return true;
}

//-----------------------------------------------------------------------------
// Очистка буферов приема и передачи
uint8_t nRF5x_UART_flush(uint8_t instance)
{
  ret_code_t status;
  
  if(instance >= UART_INSTANCE_COUNT) return false;
  
  status = nRF5x_FIFO_flush(&nRF5x_UART_rx_fifo[instance]);
  if(status != NRF_SUCCESS) return false;
  
  status = nRF5x_FIFO_flush(&nRF5x_UART_tx_fifo[instance]);
  if(status != NRF_SUCCESS) return false;
  
  return status;
}
