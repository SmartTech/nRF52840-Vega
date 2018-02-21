#ifndef _WIRISH_DIGITAL_H_
#define _WIRISH_DIGITAL_H_

#include "nRF52core.h"

enum {
  INPUT=0,
  INPUT_PULLUP,
  INPUT_PULLDOWN,
  SENSE_LOW,
  SENSE_LOW_PULLUP,
  SENSE_LOW_PULLDOWN,
  SENSE_HIGH,
  SENSE_HIGH_PULLUP,
  SENSE_HIGH_PULLDOWN,
  OUTPUT,
  OUTPUT_OD,
  WATCHER
};

enum {
  FALLING=0,
  RISING,
  CHANGE
};

typedef void (*interruptHandler)(uint8_t pin, uint8_t action);

void   pinMode(uint8 pin, uint8_t mode);

void   attachInterrupt(uint8 pin, interruptHandler handler, uint8 mode);

uint32 digitalRead(uint8 pin);
void   digitalWrite(uint8 pin, uint8 val);
void   digitalToggle(uint8 pin);
void   digitalSet(uint8 pin);
void   digitalReset(uint8 pin);
void   digitalClear(uint8 pin);

uint32 readPin(uint8 pin);
void   writePin(uint8 pin, uint8 val);
void   togglePin(uint8 pin);
void   setPin(uint8 pin);
void   resetPin(uint8 pin);
void   clearPin(uint8 pin);

#endif // _WIRISH_DIGITAL_H_
