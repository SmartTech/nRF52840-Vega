#ifndef _SPI_CLASS_H_
#define _SPI_CLASS_H_

#include "pinName.h"

#include "nRF5x_SPI.h"

#define LSBFIRST          NRF_SPI_BIT_ORDER_LSB_FIRST
#define MSBFIRST          NRF_SPI_BIT_ORDER_MSB_FIRST

#define SPI_MODE0         NRF_SPI_MODE_0
#define SPI_MODE1         NRF_SPI_MODE_1
#define SPI_MODE2         NRF_SPI_MODE_2
#define SPI_MODE3         NRF_SPI_MODE_3

#define SPI_FREQ_125K     NRF_SPI_FREQ_125K  // 125 KHz
#define SPI_FREQ_250K     NRF_SPI_FREQ_250K  // 250 KHz
#define SPI_FREQ_500K     NRF_SPI_FREQ_500K  // 500 KHz
#define SPI_FREQ_1M       NRF_SPI_FREQ_1M    // 1 MHz
#define SPI_FREQ_2M       NRF_SPI_FREQ_2M    // 2 MHz
#define SPI_FREQ_4M       NRF_SPI_FREQ_4M    // 4 MHz
#define SPI_FREQ_8M       NRF_SPI_FREQ_8M    // 8 MHz

class SPIClass {
  
  public:
    
        SPIClass();
    
        SPIClass(uint8_t _port, uint8_t _pinMOSI = PIN_MOSI, uint8_t _pinMISO = PIN_MISO, uint8_t _pinSCK = PIN_SCK);

        uint8_t transfer(uint8_t byte);
	uint8_t transfer(uint8_t* data, size_t count);
    
	void begin(void);
	void end(void);	
	
        void setNSS         (uint8_t pinNSS);
	void setBitOrder    (uint8_t bitOrder);
	void setDataMode    (uint8_t mode);
	void setClockDivider(uint8_t freq); 

  private:
    
	uint8_t port;
	uint8_t pinMOSI;
	uint8_t pinMISO;
	uint8_t pinSCK;
	uint8_t pinNSS;
	//BitOrder _order;
	//uint8_t interruptMode;
	//uint32_t interruptMask;
	//uint8_t interruptSave;
	//uint8_t inTransactionFlag;
};

#endif //_SPI_CLASS_H_