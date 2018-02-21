#include "delay.h"
#include "nrf_delay.h"

#ifdef __cplusplus
extern "C" {
#endif

static volatile uint32_t _ulTickCount=0;

uint8_t systick_inited = 0;

void systick_init() {
  if(SysTick_Config( SystemCoreClock / 1000 ) ) {
    while(1);
  }
  systick_inited = 1;
}

uint32_t millis( void ) {
  if(!systick_inited) systick_init();
  // todo: ensure no interrupts
  return _ulTickCount ;
}

// Interrupt-compatible version of micros
// Theory: repeatedly take readings of SysTick counter, millis counter and SysTick interrupt pending flag.
// When it appears that millis counter and pending is stable and SysTick hasn't rolled over, use these
// values to calculate micros. If there is a pending SysTick, add one to the millis counter in the calculation.
uint32_t micros( void ) { 
  uint32_t ticks, ticks2;
  uint32_t pend, pend2;
  uint32_t count, count2;
  
  if(!systick_inited) systick_init();

  ticks2  = SysTick->VAL;
  pend2   = !!(SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)  ;
  count2  = _ulTickCount ;

  do {
    ticks   = ticks2;
    pend    = pend2;
    count   = count2;
    ticks2  = SysTick->VAL;
    pend2   = !!(SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)  ;
    count2  = _ulTickCount ;
  } while ((pend != pend2) || (count != count2) || (ticks < ticks2));
  
  return ((count+pend) * 1000) + (((SysTick->LOAD  - ticks)*(1048576/(VARIANT_MCK/1000000)))>>20) ;
  // this is an optimization to turn a runtime division into two compile-time divisions and
  // a runtime multiplication and shift, saving a few cycles
}

void SysTick_Handler(void){
  // Increment tick count each ms
  _ulTickCount++ ;
}

void delay(uint32_t ms) {
  if(!systick_inited) systick_init();
  //nrf_delay_ms(ms);
  if( ms == 0 ) return ;
  uint32_t start = _ulTickCount ;
  do {
    yield();
  } while ( _ulTickCount - start <= (ms-1) ) ;
}

void delayMicroseconds(uint32_t usec) {
  if(!systick_inited) systick_init();
  nrf_delay_us(usec);
}

#ifdef __cplusplus
}
#endif
