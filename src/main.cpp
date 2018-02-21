#include "nRF52core.h"

#include "nRF52_UART.h"

//HardwareSerial Serial0(nRF52_UART0, LINK_RX, LINK_TX);
//HardwareSerial Serial1(nRF52_UART1, P1_01,   P1_02);

void uart_0_handler() {
  uint8_t byte;
  nRF52_UART_read (nRF52_UART0, &byte, 1);                // byte = Serial0.read();
  nRF52_UART_write(nRF52_UART1, &byte, 1);                // Serial1.write(byte);
}                                                         
                                                          
void uart_1_handler() {                                   
  uint8_t byte;                                           
  nRF52_UART_read (nRF52_UART1, &byte, 1);                // byte = Serial1.read();
  nRF52_UART_write(nRF52_UART0, &byte, 1);                // Serial0.write(byte);
}

void setup() {
  
  nRF52_UART_init(nRF52_UART0, 115200, LINK_RX, LINK_TX); // Serial0.begin(115200);
  nRF52_UART_init(nRF52_UART1, 115200, P1_01,   P1_02);   // Serial1.begin(115200);
    
  nRF52_UART_attachCallback(nRF52_UART0, uart_0_handler); // Serial0.attachCallback(uart_0_handler);
  nRF52_UART_attachCallback(nRF52_UART1, uart_1_handler); // Serial1.attachCallback(uart_1_handler);
  
  //nRF52_UART_deinit(nRF52_UART0);                       // Serial0.end();
  //nRF52_UART_deinit(nRF52_UART1);                       // Serial1.end();
  
  pinMode(LED_1, OUTPUT);
  
}


void loop() {
  
  digitalToggle(LED_1);
  
  delay(200);

}

int main()
{
  setup();   
  while(1) loop();
}
