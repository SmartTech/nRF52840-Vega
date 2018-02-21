#ifndef _HARDWARE_SERIAL_H_
#define _HARDWARE_SERIAL_H_


#include "types.h"

#include "Print.h"
#include "Stream.h"

#include "pinName.h"

#include "nRF5x_UART.h"

class HardwareSerial : public Stream {
  
  public:
  
    HardwareSerial();
    HardwareSerial(uint8_t _port, uint8_t _hfc = false);
    HardwareSerial(uint8_t _port, uint8_t _rx=PIN_RX, uint8_t _tx=PIN_TX, uint8_t _rts=PIN_RTS, uint8_t _cts=PIN_CTS, uint8_t _hfc = false);
    ~HardwareSerial();
    
    void construct(uint8_t _port);
    
    /* Set up/tear down */
    uint8_t begin(uint32 baud);
    void end();
    virtual int available(void);
    virtual int peek(void);
    virtual int read(void);
    int availableForWrite(void);
    virtual void flush(void);
    virtual size_t write(uint8_t);
    inline size_t write(unsigned long n) { return write((uint8_t)n); }
    inline size_t write(long n) { return write((uint8_t)n); }
    inline size_t write(unsigned int n) { return write((uint8_t)n); }
    inline size_t write(int n) { return write((uint8_t)n); }
    using Print::write;

    /* Pin accessors */
    int txPin(void)  { return this->tx_pin; }
    int rxPin(void)  { return this->rx_pin; }
    int rtsPin(void) { return this->rts_pin; }
    int ctsPin(void) { return this->cts_pin; }
	
    operator bool() { return true; }
  
  private:
    
    uint8_t port;
        
    uint8 rx_pin;
    uint8 tx_pin;
    uint8 rts_pin;
    uint8 cts_pin;
    
    uint8 hfc;
    
  protected:
#if 0  
    volatile uint8_t * const _ubrrh;
    volatile uint8_t * const _ubrrl;
    volatile uint8_t * const _ucsra;
    volatile uint8_t * const _ucsrb;
    volatile uint8_t * const _ucsrc;
    volatile uint8_t * const _udr;
    // Has any byte been written to the UART since begin()
    bool _written;

    volatile rx_buffer_index_t _rx_buffer_head;
    volatile rx_buffer_index_t _rx_buffer_tail;
    volatile tx_buffer_index_t _tx_buffer_head;
    volatile tx_buffer_index_t _tx_buffer_tail;	
    // Don't put any members after these buffers, since only the first
    // 32 bytes of this struct can be accessed quickly using the ldd
    // instruction.
    unsigned char _rx_buffer[SERIAL_RX_BUFFER_SIZE];
    unsigned char _tx_buffer[SERIAL_TX_BUFFER_SIZE];	
#endif
  
};

#endif // _HARDWARE_SERIAL_H_