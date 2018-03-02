#include "USBSerial.h"

#include "string.h"
#include "stdint.h"


/*
 * USBSerial interface
 */
#define USB_TIMEOUT 50

USBSerial::USBSerial(void) {

}

void USBSerial::begin(void) {
    nRF5x_USB_CDC_init();
}

void USBSerial::begin(unsigned long ignoreBaud) 
{
    volatile unsigned long removeCompilerWarningsIgnoreBaud=ignoreBaud;
    ignoreBaud=removeCompilerWarningsIgnoreBaud;
    begin();
}

void USBSerial::begin(unsigned long ignoreBaud, uint8_t ignore)
{
    volatile unsigned long removeCompilerWarningsIgnoreBaud=ignoreBaud;
    volatile uint8_t removeCompilerWarningsIgnore=ignore;
    ignoreBaud=removeCompilerWarningsIgnoreBaud;
    ignore=removeCompilerWarningsIgnore;
    begin();
}

void USBSerial::end(void) {
    nRF5x_USB_CDC_deinit();
}

void USBSerial::attachCallback(nRF5x_USB_CDC_eventCallback function) {
    nRF5x_USB_CDC_attachCallback(function);
}

size_t USBSerial::write(uint8 ch) {
    nRF5x_USB_CDC_write((uint8_t*)ch, 1);
    return 1;
}

size_t USBSerial::write(const char *str) {
    //nRF5x_USB_CDC_print(str);
    nRF5x_USB_CDC_write((uint8_t*)str, strlen(str));
    return 1;
}

size_t USBSerial::write(const uint8 *buf, uint32 len) {
    nRF5x_USB_CDC_write((uint8_t*)buf, len);
    return 1;
}

int USBSerial::available(void) {
    //return usb_cdcacm_data_available();
    return -1;
}

int USBSerial::peek(void) {
    //uint8 b;
    //if (usb_cdcacm_peek(&b, 1)==1) {
    //    return b;
    //}
    //else {
        return -1;
    //}
}

void USBSerial::flush(void) {
    uint8 b;
    while(this->available()) {
        this->read(&b, 1);
    }
    return;
}

uint32 USBSerial::read(uint8 * buf, uint32 len) {
    uint32 rxed = 0;
    while (rxed < len) {
        //rxed += usb_cdcacm_rx(buf + rxed, len - rxed);
    }
    return rxed;
}

size_t USBSerial::readBytes(char *buf, const size_t& len) {
    size_t rxed=0;
    //unsigned long startMillis;
    //startMillis = millis();
    //if (len <= 0) return 0;
    //do {
    //    rxed += usb_cdcacm_rx((uint8 *)buf + rxed, len - rxed);
    //    if (rxed == len) return rxed;
    //} while(millis() - startMillis < _timeout);
    return rxed;
}

int USBSerial::read(void) {
    //uint8 b;
    //if (usb_cdcacm_rx(&b, 1)==0) {
        return -1;
    //}
    //else {
    //    return b;
    //}
}

uint8 USBSerial::pending(void) {
    //return usb_cdcacm_get_pending();
    return 0;
}

uint8 USBSerial::getDTR(void) {
    //return usb_cdcacm_get_dtr();
    return 0;
}

uint8 USBSerial::getRTS(void) {
    //return usb_cdcacm_get_rts();
    return 0;
}

USBSerial::operator bool() {
    //return usb_is_connected(USBLIB) && usb_is_configured(USBLIB) && usb_cdcacm_get_dtr();
    return 0;
}

