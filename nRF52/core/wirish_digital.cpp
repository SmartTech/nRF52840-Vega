#include "wirish_digital.h"

//-----------------------------------------------------

void pinMode(uint8 pin, uint8_t mode) {
  switch(mode) {
    case INPUT               : nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_NOPULL);                                  break;
    case INPUT_PULLUP        : nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLUP);                                  break;
    case INPUT_PULLDOWN      : nrf_gpio_cfg_input(pin, NRF_GPIO_PIN_PULLDOWN);                                break;
    case SENSE_LOW           : nrf_gpio_cfg_sense_input(pin, NRF_GPIO_PIN_NOPULL,   NRF_GPIO_PIN_SENSE_LOW);  break;
    case SENSE_LOW_PULLUP    : nrf_gpio_cfg_sense_input(pin, NRF_GPIO_PIN_PULLUP,   NRF_GPIO_PIN_SENSE_LOW);  break;
    case SENSE_LOW_PULLDOWN  : nrf_gpio_cfg_sense_input(pin, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_LOW);  break;
    case SENSE_HIGH          : nrf_gpio_cfg_sense_input(pin, NRF_GPIO_PIN_NOPULL,   NRF_GPIO_PIN_SENSE_HIGH); break;
    case SENSE_HIGH_PULLUP   : nrf_gpio_cfg_sense_input(pin, NRF_GPIO_PIN_PULLUP,   NRF_GPIO_PIN_SENSE_HIGH); break;
    case SENSE_HIGH_PULLDOWN : nrf_gpio_cfg_sense_input(pin, NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_SENSE_HIGH); break;
    case OUTPUT              : nrf_gpio_cfg_output(pin);
    case OUTPUT_OD           : break;
    case WATCHER             : break;
    default: break;
  }
}

void gpiote_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
  //interruptHandler();
}

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
   // nrf_drv_gpiote_out_toggle(PIN_OUT);
}

void attachInterrupt(uint8_t pin, interruptHandler handler, uint8_t mode) {
  nrf_drv_gpiote_in_config_t in_config;
  in_config.is_watcher  = false;
  in_config.hi_accuracy = true;
  in_config.pull        = NRF_GPIO_PIN_PULLUP;
  switch(mode) {
    case FALLING : {
      in_config.sense = NRF_GPIOTE_POLARITY_HITOLO;
    } break;
    case RISING : {
      in_config.sense = NRF_GPIOTE_POLARITY_LOTOHI;
    } break;
    case CHANGE : {
      in_config.sense = NRF_GPIOTE_POLARITY_TOGGLE;
    } break;
    default: break;
  }
  if(!nrf_drv_gpiote_is_init()) nrf_drv_gpiote_init();
  //nrf_drv_gpiote_in_init(pin, &in_config, handler);
}

//-----------------------------------------------------

uint32 digitalRead(uint8 pin) {
  return nrf_gpio_pin_read(pin) ? HIGH : LOW;
}

void digitalWrite(uint8 pin, uint8 val) {
    nrf_gpio_pin_write(pin, val);
}

void digitalToggle(uint8 pin) {
    nrf_gpio_pin_toggle(pin);
}

void digitalSet(uint8 pin) {
    nrf_gpio_pin_set(pin);
}

void digitalReset(uint8 pin) {
    nrf_gpio_pin_clear(pin);
}

void digitalClear(uint8 pin) {
    nrf_gpio_pin_clear(pin);
}

//-----------------------------------------------------

uint32 readPin(uint8 pin) {
  return nrf_gpio_pin_read(pin) ? HIGH : LOW;
}

void   writePin(uint8 pin, uint8 val) {
    nrf_gpio_pin_write(pin, val);
}

void   togglePin(uint8 pin) {
    nrf_gpio_pin_toggle(pin);
}

void   setPin(uint8 pin) {
    nrf_gpio_pin_set(pin);
}

void   clearPin(uint8 pin) {
    nrf_gpio_pin_clear(pin);
}

void   resetPin(uint8 pin) {
    nrf_gpio_pin_clear(pin);
}

//-----------------------------------------------------
