#ifndef _WIRISH_USB_SERIAL_H_
#define _WIRISH_USB_SERIAL_H_

#include "Print.h"
#include "Stream.h"

#include "nRF5x_USB_CDC.h"

class USBSerial : public Stream {
  
private:
      
public:
  
    USBSerial(void);

    void begin(void);

    void begin(unsigned long);
    void begin(unsigned long, uint8_t);
    void end(void);
    
    void attachCallback(nRF5x_USB_CDC_eventCallback function);

    virtual int available(void);

    size_t readBytes(char *buf, const size_t& len);
    uint32 read(uint8 * buf, uint32 len);

    virtual int peek(void);
    virtual int read(void);
    int availableForWrite(void);
    virtual void flush(void);

    size_t write(uint8);
    size_t write(const char *str);
    size_t write(const uint8*, uint32);

    uint8 getRTS();
    uint8 getDTR();
    uint8 pending();
    
    uint8_t isConnected();

    operator bool();
    
    using Print::write;

    //uint8 isConnected() __attribute__((deprecated("Use !Serial instead"))) { return (bool) *this; }

    
};

#endif
