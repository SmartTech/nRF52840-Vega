#include "HardwareSerial.h"

HardwareSerial::HardwareSerial() :
  port(0), rx_pin(PIN_RX), tx_pin(PIN_TX), rts_pin(PIN_RTS), cts_pin(PIN_CTS), hfc(false)
{
  construct(0);
}

HardwareSerial::HardwareSerial(uint8_t _port, uint8_t _hfc) :
  port(_port), rx_pin(PIN_RX), tx_pin(PIN_TX), rts_pin(PIN_RTS), cts_pin(PIN_CTS)
{
  construct(_port);
}

HardwareSerial::HardwareSerial(uint8_t _port, uint8_t _rx, uint8_t _tx, uint8_t _rts, uint8_t _cts, uint8_t _hfc) :
  port(_port), rx_pin(_rx), tx_pin(_tx), rts_pin(_rts), cts_pin(_cts), hfc(_hfc)
{
  construct(_port);
}

void HardwareSerial::construct(uint8_t _port) {
  if(_port>1) return;
  port = _port;
}

HardwareSerial::~HardwareSerial()
{
}

uint8_t HardwareSerial::begin(uint32 baud) {
  nRF5x_UART_init(port, baud, rx_pin, tx_pin);
  return NRF_SUCCESS;
}

void HardwareSerial::end(void)
{
    nRF5x_UART_deinit(port);
}

int HardwareSerial::available(void)
{
    return nRF5x_UART_available(port);
}

int HardwareSerial::peek(void)
{
    //return usart_peek(this->usart_device);
    return -1;
}

int HardwareSerial::availableForWrite(void)
{
  return 1;
}

int HardwareSerial::read(void)
{
    if(nRF5x_UART_available(port)) {
      uint8_t byte;
      if(nRF5x_UART_read(port, &byte, 1)) return byte;
    }
    return -1;
}

size_t HardwareSerial::write(uint8_t txByte) {
  uint8_t tx_buffer[1];
  tx_buffer[0] = txByte;
  nRF5x_UART_write(port, tx_buffer, 1);
  return 1;
}

void HardwareSerial::flush(void) {
  nRF5x_UART_flush(port);
}
