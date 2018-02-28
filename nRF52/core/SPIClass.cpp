#include "SPIClass.h"

SPIClass::SPIClass() 
{  
  SPIClass(0);
}

SPIClass::SPIClass(uint8_t _port, uint8_t _pinMOSI, uint8_t _pinMISO, uint8_t _pinSCK) :
  port(_port), pinMOSI(_pinMOSI), pinMISO(_pinMISO), pinSCK(_pinSCK), pinNSS(255)
{
}

void SPIClass::begin()
{
  nRF5x_SPI_begin(port, pinMOSI, pinMISO, pinSCK);
}


void SPIClass::end()
{
  nRF5x_SPI_deinit(port);
}

void SPIClass::setNSS(uint8_t pinNSS)
{
  nRF5x_SPI_setNSS(port, pinNSS);
}

void SPIClass::setBitOrder(uint8_t bitOrder)
{
  nRF5x_SPI_setBitOrder(port, bitOrder);
}

void SPIClass::setDataMode(uint8_t mode)
{
  nRF5x_SPI_setMode(port, mode);
}


void SPIClass::setClockDivider(uint8_t freq)
{
  nRF5x_SPI_setFreq(port, (nrf_drv_spi_frequency_t)freq);
}

uint8_t SPIClass::transfer(uint8_t byte) {
  return this->transfer(&byte, 1);
}

uint8_t SPIClass::transfer(uint8_t* data, size_t count) {
  uint8_t result;
  for(size_t i=0; i<count; i++) {
    result = nRF5x_SPI_transfer(port, data[i]);
  }
  return result;
}
