#ifndef _PINNAME_H_
#define _PINNAME_H_

#include "nRF52core.h"


/* -------------------------------------------------------------
 *  Port 0 pins mapping
 * -----------------------------------------------------------*/
 
#define P0_00 NRF_GPIO_PIN_MAP(0, 0) // I/O, XL1 (32.768 kHz Crystal)
#define P0_01 NRF_GPIO_PIN_MAP(0, 1) // I/O, XL2 (32.768 kHz Crystal)
#define P0_02 NRF_GPIO_PIN_MAP(0, 2) // I/O, AIN0
#define P0_03 NRF_GPIO_PIN_MAP(0, 3) // I/O, AIN1
#define P0_04 NRF_GPIO_PIN_MAP(0, 4) // I/O, AIN2
#define P0_05 NRF_GPIO_PIN_MAP(0, 5) // I/O, AIN3
#define P0_06 NRF_GPIO_PIN_MAP(0, 6) // I/O
#define P0_07 NRF_GPIO_PIN_MAP(0, 7) // I/O
#define P0_08 NRF_GPIO_PIN_MAP(0, 8) // I/O
#define P0_09 NRF_GPIO_PIN_MAP(0, 9) // I/O, NFC1 (NFC antenna)
#define P0_10 NRF_GPIO_PIN_MAP(0,10) // I/O, NFC2 (NFC antenna)
#define P0_11 NRF_GPIO_PIN_MAP(0,11) // I/O
#define P0_12 NRF_GPIO_PIN_MAP(0,12) // I/O
#define P0_13 NRF_GPIO_PIN_MAP(0,13) // I/O
#define P0_14 NRF_GPIO_PIN_MAP(0,14) // I/O
#define P0_15 NRF_GPIO_PIN_MAP(0,15) // I/O
#define P0_16 NRF_GPIO_PIN_MAP(0,16) // I/O
#define P0_17 NRF_GPIO_PIN_MAP(0,17) // I/O
#define P0_18 NRF_GPIO_PIN_MAP(0,18) // I/O, nRESET (Configurable as system RESET)
#define P0_19 NRF_GPIO_PIN_MAP(0,19) // I/O
#define P0_20 NRF_GPIO_PIN_MAP(0,20) // I/O
#define P0_21 NRF_GPIO_PIN_MAP(0,21) // I/O
#define P0_22 NRF_GPIO_PIN_MAP(0,22) // I/O
#define P0_23 NRF_GPIO_PIN_MAP(0,23) // I/O
#define P0_24 NRF_GPIO_PIN_MAP(0,24) // I/O
#define P0_25 NRF_GPIO_PIN_MAP(0,25) // I/O
#define P0_26 NRF_GPIO_PIN_MAP(0,26) // I/O
#define P0_27 NRF_GPIO_PIN_MAP(0,27) // I/O
#define P0_28 NRF_GPIO_PIN_MAP(0,28) // I/O, AIN4
#define P0_29 NRF_GPIO_PIN_MAP(0,29) // I/O, AIN5
#define P0_30 NRF_GPIO_PIN_MAP(0,30) // I/O, AIN6
#define P0_31 NRF_GPIO_PIN_MAP(0,31) // I/O, AIN7

/* -------------------------------------------------------------
 *  Port 1 pins mapping
 * -----------------------------------------------------------*/
 
#define P1_01 NRF_GPIO_PIN_MAP(1, 1) // I/O
#define P1_02 NRF_GPIO_PIN_MAP(1, 2) // I/O
#define P1_03 NRF_GPIO_PIN_MAP(1, 3) // I/O
#define P1_04 NRF_GPIO_PIN_MAP(1, 4) // I/O
#define P1_05 NRF_GPIO_PIN_MAP(1, 5) // I/O
#define P1_06 NRF_GPIO_PIN_MAP(1, 6) // I/O
#define P1_07 NRF_GPIO_PIN_MAP(1, 7) // I/O
#define P1_08 NRF_GPIO_PIN_MAP(1, 8) // I/O
#define P1_09 NRF_GPIO_PIN_MAP(1, 9) // I/O
#define P1_10 NRF_GPIO_PIN_MAP(1,10) // I/O
#define P1_11 NRF_GPIO_PIN_MAP(1,11) // I/O
#define P1_12 NRF_GPIO_PIN_MAP(1,12) // I/O
#define P1_13 NRF_GPIO_PIN_MAP(1,13) // I/O
#define P1_14 NRF_GPIO_PIN_MAP(1,14) // I/O
#define P1_15 NRF_GPIO_PIN_MAP(1,15) // I/O

/* -------------------------------------------------------------
 *  Analog pins mapping
 * -----------------------------------------------------------*/
 
#define A0    P0_02                  // AIN0
#define A1    P0_03                  // AIN1
#define A2    P0_04                  // AIN2
#define A3    P0_05                  // AIN3
#define A4    P0_28                  // AIN4
#define A5    P0_29                  // AIN5
#define A6    P0_30                  // AIN6
#define A7    P0_31                  // AIN7

/* -------------------------------------------------------------
 *  PCA10056 board pins mapping
 * ------------------------------------------------------------*/
 
#define PIN_RX    SER_CON_RX_PIN         // P1_13 // NRF_GPIO_PIN_MAP(1,13)
#define PIN_TX    SER_CON_TX_PIN         // P1_14 // NRF_GPIO_PIN_MAP(1,14)
#define PIN_RTS   SER_CON_RTS_PIN        // P1_15 // NRF_GPIO_PIN_MAP(1,15)
#define PIN_CTS   SER_CON_CTS_PIN        // P0_02 // NRF_GPIO_PIN_MAP(0, 2)

#define LINK_RX   P0_08                  // P0_02 // NRF_GPIO_PIN_MAP(0, 2)
#define LINK_TX   P0_06                  // P0_02 // NRF_GPIO_PIN_MAP(0, 2)

#define PIN_SDA   P0_31
#define PIN_SCL   P0_31

#define PIN_MOSI  SER_APP_SPIM0_MOSI_PIN // P0_27 // NRF_GPIO_PIN_MAP(0,27)
#define PIN_MISO  SER_APP_SPIM0_MISO_PIN // P0_02 // NRF_GPIO_PIN_MAP(0, 2) 
#define PIN_SCK   SER_APP_SPIM0_SCK_PIN  // P0_26 // NRF_GPIO_PIN_MAP(0,26)
#define PIN_NSS   SER_APP_SPIM0_SS_PIN   // P1_13 // NRF_GPIO_PIN_MAP(1,13)

#endif // _PINNAME_H_
