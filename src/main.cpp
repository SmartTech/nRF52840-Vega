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
    digitalWrite(LORA_NSS, LOW);                  // устанавливаем линию NSS
    bitClear(address, 7);                      // очищаем бит 7 для записи в регистр
    // Вариант #1
    nRF5x_SPI_transfer(nRF5x_SPI0, address);   // огранизуем передачу для указания адреса чтения (ответ не важен)
    value = nRF5x_SPI_transfer(nRF5x_SPI0, 0); // огранизуем передачу для приема ответа (значение передачи не важно)
    // Вариант #2
    //uint8_t rx;
    //nRF5x_SPI_writeRead(nRF5x_SPI0, &address, 1, &rx, 1);
    //nRF5x_SPI_writeRead(nRF5x_SPI0, 0, 1, &rx, 1);
    //value = rx;
    // Вариант #3 - через класс SPIClass
    //SPI.transfer(address);
    //value = SPI.transfer(0);
    digitalWrite(LORA_NSS, HIGH);                 // отпускаем линию NSS
    return value;
}

//===========================================================================
// инициализации UART
void UART_init() {
  
  nRF5x_UART_init(nRF5x_UART0, 115200, LINK_RX, LINK_TX); // Serial0.begin(115200);
  nRF5x_UART_init(nRF5x_UART1, 115200, P1_01,   P1_02);   // Serial1.begin(115200);
  
  // подключение callback функци по приему байта
  nRF5x_UART_attachCallback(nRF5x_UART0, uart_0_handler); // Serial0.attachCallback(uart_0_handler);
  nRF5x_UART_attachCallback(nRF5x_UART1, uart_1_handler); // Serial1.attachCallback(uart_1_handler);
  
  // деинициализация UART
  //nRF52_UART_deinit(nRF52_UART0);                       // Serial0.end();
  //nRF52_UART_deinit(nRF52_UART1);                       // Serial1.end();
  
  //---------------------------------------------------------------------------
  // Инициализация через класс HardwareSerial
  // Serial0.begin(115200);
  // Serial1.begin(115200);
}

//===========================================================================
// инициализации SPI
void SPI_init() {
  
  //===========================================================================
  // Вариант #1 инициализации SPI (структуры конфигурации хранятся в библиотеке)
  nRF5x_SPI_begin(nRF5x_SPI0, P1_13, P1_14, P1_15);
  //Методы переконфигурации:
  //nRF5x_SPI_setNSS     (nRF5x_SPI0, P0_04);
  //nRF5x_SPI_setFreq    (nRF5x_SPI0, NRF_DRV_SPI_FREQ_4M);
  //nRF5x_SPI_setMode    (nRF5x_SPI0, NRF_DRV_SPI_MODE_0);
  //nRF5x_SPI_setBitOrder(nRF5x_SPI0, NRF_DRV_SPI_BIT_ORDER_MSB_FIRST);
  
  //---------------------------------------------------------------------------
  // Вариант #2 инициализации SPI через заданную структуру конфигурации
  //nRF5x_spi_config_t spiConfig = nRF5x_SPI_DEFAULT_CONFIG;
  //spiConfig.mosi_pin = P1_13;
  //spiConfig.miso_pin = P1_14;
  //spiConfig.sck_pin  = P1_15;
  //spiConfig.mode     = NRF_DRV_SPI_MODE_0;
  // и другие параметры...
  //nRF5x_SPI_init(nRF5x_SPI0, &spiConfig);
  
  //---------------------------------------------------------------------------
  // Вариант #3 Инициализация через класс SPIClass
  //SPI.begin();
  //SPI.setNSS(LORA_NSS);
  //SPI.setDataMode(NRF_DRV_SPI_MODE_0);
}

//===========================================================================
// инициализации SPI
void TWI_init() {
  // Инициализация шины I2C
  nRF5x_TWI_begin(nRF5x_TWI0, PIN_SDA, PIN_SCL);
  nRF5x_TWI_setFreq(nRF5x_TWI0, NRF_TWI_FREQ_100K);
  // Сканирование шины I2C
  uint8_t devices = 0;
  for(uint8_t addr=1; addr<=127; addr++) {
    uint8_t code = nRF5x_TWI_beginTransmission(nRF5x_TWI0, addr);
    //uint8_t code = nRF5x_TWI_endTransmission(nRF5x_TWI0);
    if(code) {
      // найдено уст-во по опрашиваемому адресу
      devices++;
      nRF5x_UART_write(nRF5x_UART0, &addr, 1);
    }
    delay(10);
  }
  if(!devices) {
    // устройства не найдены
  }
}

//=============================================================================
// Инициализация --------------------------------------------------------------
void setup() {
  
  UART_init();
  
  SPI_init();
  
  TWI_init();
  
  nRF5x_RTC_resetTime(2);                            // обнуляем время
  nRF5x_RTC_ALARM_t alarm = nRF5x_RTC_DEFAULT_ALARM; // объявляем аларм
  alarm.time.second = 15;                            // задаем секунды аларма
  nRF5x_RTC_setAlarm(alarm, wake);                   // устанавливаем аларм
  nRF5x_RTC_enableTime(2);                           // активируем счетчик
  
  pinMode(LED_1, OUTPUT);
  
  // Настройка пинов управления NSS и RST мекросхемы SX1272
  pinMode(LORA_NSS, OUTPUT); digitalWrite(LORA_NSS, HIGH); // NSS
  pinMode(LORA_RST, OUTPUT); digitalWrite(LORA_RST, HIGH); // RST
  SX1272_reset(); // аппаратный сброс чипа SX1272
  
}

// Главный цикл ---------------------------------------------------------------
void loop() {
  
  // изменение состояния светодиода
  digitalToggle(LED_1); // так же можно использовать togglePin(LED_1);
  
  // чтение версии(ID) микросхемы SX1272 и вывод информации в UART0
  uint8_t version = SX1272_readRegister(0x42);
  nRF5x_UART_write(nRF5x_UART0, &version, 1);
  
  uint8_t second = nRF5x_RTC_getSecond();
  nRF5x_UART_write(nRF5x_UART0, &second, 1);
  
  // Отправка в uart через класс HardwareSerial
  //Serial0.println("Lora [version] = " + String(version));
  
  // задержка в 1000 микросекунд (основана на системном таймере)
  delay(1000);
  
  // Контроль перехода в сон
  static int sleepAfter   = 6;
  static int sleepCount   = 1;
  static int sleepCounter = 0;
  if(sleepCounter++ > sleepAfter * sleepCount) {
    sleepCount++;
    sleep();
  }

}

//=============================================================================
// Псевдо-сон -----------------------------------------------------------------
void sleep() {
  // Отключаем всю переферию
  nRF5x_UART_deinit(nRF5x_UART0);
  nRF5x_SPI_deinit(nRF5x_SPI0);
  nRF5x_TWI_deinit(nRF5x_TWI0);
  // Отключаем все пины
  pinMode(LED_1, INPUT);
  isSleep = true;
  // Ждем выхода из псевдо-сна по флагу
  while(isSleep) {};
  // Note: энергопотребление обеспечивается за счет
  //       отключения всей переферии
  setup();
}

// Псевдо-просыпание ----------------------------------------------------------
void wake() {
  isSleep = false;
}

//=============================================================================
// Запуск контроллера ---------------------------------------------------------
int main()
{
  setup();         // инициализируем необходимую переферию
  while(1) loop(); // переходим к главному циклу
}
