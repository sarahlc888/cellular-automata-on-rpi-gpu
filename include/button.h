#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "ringbuffer.h"

enum button_events{
  BUTTON_PRESS = 0,
  BUTTON_HOLD = 1
};
typedef enum button_events button_event_t;

void button_init(unsigned pin);

int get_button(unsigned pin);

rb_t* get_button_rb(unsigned pin);

