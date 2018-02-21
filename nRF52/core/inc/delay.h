#ifndef _DELAY_
#define _DELAY_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define VARIANT_MCK			  (64000000ul)

extern uint32_t millis( void ) ;

extern uint32_t micros( void ) ;

extern void delay( uint32_t dwMs ) ;

extern void delayMicroseconds(uint32_t usec);

#define yield()

#ifdef __cplusplus
}
#endif

#endif /* _DELAY_ */
