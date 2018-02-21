#ifndef _NRF52_CORE_H_
#define _NRF52_CORE_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "types.h"

#include "nrf.h"

#include "pca10056.h"

#include "nrf_gpio.h"
#include "nrf_gpiote.h"
#include "nrf_delay.h"

#include "nrf_pwr_mgmt.h"
#include "nrf_drv_power.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "nrf_drv_clock.h"
#include "nrf_drv_gpiote.h"

#define clockCyclesPerMicrosecond()  ( SystemCoreClock / 1000000L )
#define clockCyclesToMicroseconds(a) ( ((a) * 1000L) / (SystemCoreClock / 1000L) )
#define microsecondsToClockCycles(a) ( (a) * (SystemCoreClock / 1000000L) )

#include "delay.h"

#include "pinName.h"
#include "wirish_digital.h"
#include "HardwareSerial.h"

#endif // _NRF52_CORE_H_
