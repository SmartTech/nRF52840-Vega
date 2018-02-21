#include "USBSerial.h"

#include "string.h"
#include "stdint.h"


/*
 * USBSerial interface
 */
#define USB_TIMEOUT 50

bool USBSerial::_hasBegun = false;

USBSerial::USBSerial(void) {

}

void USBSerial::begin(void) {
	
    if (_hasBegun) return;
    _hasBegun = true;
    //usb_cdcacm_enable(BOARD_USB_DISC_DEV, (uint8_t)BOARD_USB_DISC_BIT);
    //usb_cdcacm_set_hooks(USB_CDCACM_HOOK_RX, rxHook);
    //usb_cdcacm_set_hooks(USB_CDCACM_HOOK_IFACE_SETUP, ifaceSetupHook);

}

//Roger Clark. Two new begin functions has been added so that normal Arduino Sketches that use Serial.begin(xxx) will compile.
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
    //usb_cdcacm_disable(BOARD_USB_DISC_DEV, (uint8_t)BOARD_USB_DISC_BIT);
    //usb_cdcacm_remove_hooks(USB_CDCACM_HOOK_RX | USB_CDCACM_HOOK_IFACE_SETUP);
    _hasBegun = false;
}

size_t USBSerial::write(uint8 ch) {
    size_t n = 0;
    this->write(&ch, 1);
    return n;
}

size_t USBSerial::write(const char *str) {
    size_t n = 0;
    this->write((const uint8*)str, strlen(str));
    return n;
}

size_t USBSerial::write(const uint8 *buf, uint32 len) {
    size_t n = 0;
    //if (!buf || !usb_is_connected(USBLIB) || !usb_is_configured(USBLIB)) {
    //    return 0;
    //}
    //uint32 txed = 0;
    //while (txed < len) {
    //    txed += usb_cdcacm_tx((const uint8*)buf + txed, len - txed);
    //}
    return n;
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

