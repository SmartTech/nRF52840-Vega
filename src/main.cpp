#include "nRF52core.h"
#include "nRF5x_UART.h"
#include "nRF5x_SPI.h"
#include "nRF5x_TWI.h"
#include "nRF5x_RTC.h"

//HardwareSerial Serial0(nRF52_UART0, LINK_RX, LINK_TX);
//HardwareSerial Serial1(nRF52_UART1, P1_01,   P1_02);

//SPIClass SPI(nRF5x_SPI0, LORA_MOSI, LORA_MISO, LORA_SCK);

void sleep(void);
void wake(void);
uint8_t isSleep = false;

void uart_0_handler() {
  uint8_t byte;
  nRF5x_UART_read (nRF5x_UART0, &byte, 1);                // byte = Serial0.read();
  nRF5x_UART_write(nRF5x_UART1, &byte, 1);                // Serial1.write(byte);
}

void uart_1_handler() {
  uint8_t byte;                                           
  nRF5x_UART_read (nRF5x_UART1, &byte, 1);                // byte = Serial1.read();
  nRF5x_UART_write(nRF5x_UART0, &byte, 1);                // Serial0.write(byte);
}

void SX1272_reset() {
  digitalWrite(LORA_RST, HIGH);
  delay(10);
  digitalWrite(LORA_RST, LOW);
  delay(10);
}

uint8_t SX1272_readRegister(uint8_t address) {
    uint8_t value = 0;                         // 
    digitalWrite(LORA_NSS, LOW);                  // ������������� ����� NSS
    bitClear(address, 7);                      // ������� ��� 7 ��� ������ � �������
    // ������� #1
    nRF5x_SPI_transfer(nRF5x_SPI0, address);   // ���������� �������� ��� �������� ������ ������ (����� �� �����)
    value = nRF5x_SPI_transfer(nRF5x_SPI0, 0); // ���������� �������� ��� ������ ������ (�������� �������� �� �����)
    // ������� #2
    //uint8_t rx;
    //nRF5x_SPI_writeRead(nRF5x_SPI0, &address, 1, &rx, 1);
    //nRF5x_SPI_writeRead(nRF5x_SPI0, 0, 1, &rx, 1);
    //value = rx;
    // ������� #3 - ����� ����� SPIClass
    //SPI.transfer(address);
    //value = SPI.transfer(0);
    digitalWrite(LORA_NSS, HIGH);                 // ��������� ����� NSS
    return value;
}

//===========================================================================
// ������������� UART
void UART_init() {
  
  nRF5x_UART_init(nRF5x_UART0, 115200, LINK_RX, LINK_TX); // Serial0.begin(115200);
  nRF5x_UART_init(nRF5x_UART1, 115200, P1_01,   P1_02);   // Serial1.begin(115200);
  
  // ����������� callback ������ �� ������ �����
  nRF5x_UART_attachCallback(nRF5x_UART0, uart_0_handler); // Serial0.attachCallback(uart_0_handler);
  nRF5x_UART_attachCallback(nRF5x_UART1, uart_1_handler); // Serial1.attachCallback(uart_1_handler);
  
  // ��������������� UART
  //nRF52_UART_deinit(nRF52_UART0);                       // Serial0.end();
  //nRF52_UART_deinit(nRF52_UART1);                       // Serial1.end();
  
  //---------------------------------------------------------------------------
  // ������������� ����� ����� HardwareSerial
  // Serial0.begin(115200);
  // Serial1.begin(115200);
}

//===========================================================================
// ������������� SPI
void SPI_init() {
  
  //===========================================================================
  // ������� #1 ������������� SPI (��������� ������������ �������� � ����������)
  nRF5x_SPI_begin(nRF5x_SPI0, P1_13, P1_14, P1_15);
  //������ ����������������:
  //nRF5x_SPI_setNSS     (nRF5x_SPI0, P0_04);
  //nRF5x_SPI_setFreq    (nRF5x_SPI0, NRF_DRV_SPI_FREQ_4M);
  //nRF5x_SPI_setMode    (nRF5x_SPI0, NRF_DRV_SPI_MODE_0);
  //nRF5x_SPI_setBitOrder(nRF5x_SPI0, NRF_DRV_SPI_BIT_ORDER_MSB_FIRST);
  
  //---------------------------------------------------------------------------
  // ������� #2 ������������� SPI ����� �������� ��������� ������������
  //nRF5x_spi_config_t spiConfig = nRF5x_SPI_DEFAULT_CONFIG;
  //spiConfig.mosi_pin = P1_13;
  //spiConfig.miso_pin = P1_14;
  //spiConfig.sck_pin  = P1_15;
  //spiConfig.mode     = NRF_DRV_SPI_MODE_0;
  // � ������ ���������...
  //nRF5x_SPI_init(nRF5x_SPI0, &spiConfig);
  
  //---------------------------------------------------------------------------
  // ������� #3 ������������� ����� ����� SPIClass
  //SPI.begin();
  //SPI.setNSS(LORA_NSS);
  //SPI.setDataMode(NRF_DRV_SPI_MODE_0);
}

//===========================================================================
// ������������� SPI
void TWI_init() {
  // ������������� ���� I2C
  nRF5x_TWI_begin(nRF5x_TWI0, PIN_SDA, PIN_SCL);
  nRF5x_TWI_setFreq(nRF5x_TWI0, NRF_TWI_FREQ_100K);
  // ������������ ���� I2C
  uint8_t devices = 0;
  for(uint8_t addr=1; addr<=127; addr++) {
    uint8_t code = nRF5x_TWI_beginTransmission(nRF5x_TWI0, addr);
    //uint8_t code = nRF5x_TWI_endTransmission(nRF5x_TWI0);
    if(code) {
      // ������� ���-�� �� ������������� ������
      devices++;
      nRF5x_UART_write(nRF5x_UART0, &addr, 1);
    }
    delay(10);
  }
  if(!devices) {
    // ���������� �� �������
  }
}

//=============================================================================
// ������������� --------------------------------------------------------------
void setup() {
  
  UART_init();
  
  SPI_init();
  
  TWI_init();
  
  nRF5x_RTC_resetTime(2);                            // �������� �����
  nRF5x_RTC_ALARM_t alarm = nRF5x_RTC_DEFAULT_ALARM; // ��������� �����
  alarm.time.second = 15;                            // ������ ������� ������
  nRF5x_RTC_setAlarm(alarm, wake);                   // ������������� �����
  nRF5x_RTC_enableTime(2);                           // ���������� �������
  
  pinMode(LED_1, OUTPUT);
  
  // ��������� ����� ���������� NSS � RST ���������� SX1272
  pinMode(LORA_NSS, OUTPUT); digitalWrite(LORA_NSS, HIGH); // NSS
  pinMode(LORA_RST, OUTPUT); digitalWrite(LORA_RST, HIGH); // RST
  SX1272_reset(); // ���������� ����� ���� SX1272
  
}

// ������� ���� ---------------------------------------------------------------
void loop() {
  
  // ��������� ��������� ����������
  digitalToggle(LED_1); // ��� �� ����� ������������ togglePin(LED_1);
  
  // ������ ������(ID) ���������� SX1272 � ����� ���������� � UART0
  uint8_t version = SX1272_readRegister(0x42);
  nRF5x_UART_write(nRF5x_UART0, &version, 1);
  
  uint8_t second = nRF5x_RTC_getSecond();
  nRF5x_UART_write(nRF5x_UART0, &second, 1);
  
  // �������� � uart ����� ����� HardwareSerial
  //Serial0.println("Lora [version] = " + String(version));
  
  // �������� � 1000 ����������� (�������� �� ��������� �������)
  delay(1000);
  
  // �������� �������� � ���
  static int sleepAfter   = 6;
  static int sleepCount   = 1;
  static int sleepCounter = 0;
  if(sleepCounter++ > sleepAfter * sleepCount) {
    sleepCount++;
    sleep();
  }

}

//=============================================================================
// ������-��� -----------------------------------------------------------------
void sleep() {
  // ��������� ��� ���������
  nRF5x_UART_deinit(nRF5x_UART0);
  nRF5x_SPI_deinit(nRF5x_SPI0);
  nRF5x_TWI_deinit(nRF5x_TWI0);
  // ��������� ��� ����
  pinMode(LED_1, INPUT);
  isSleep = true;
  // ���� ������ �� ������-��� �� �����
  while(isSleep) {};
  // Note: ����������������� �������������� �� ����
  //       ���������� ���� ���������
  setup();
}

// ������-���������� ----------------------------------------------------------
void wake() {
  isSleep = false;
}

//=============================================================================
// ������ ����������� ---------------------------------------------------------
int main()
{
  setup();         // �������������� ����������� ���������
  while(1) loop(); // ��������� � �������� �����
}
