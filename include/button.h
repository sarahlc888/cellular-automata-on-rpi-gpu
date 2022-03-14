#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "ringbuffer.h"

enum button_events{
  BUTTON_PRESS = 0,
  BUTTON_HOLD = 1,
  BUTTON_EMPTY = -1
};

#define MAIN_BUTTON 21

#define WHITE_BUTTON 20
#define BLACK_BUTTON 16
#define RED_BUTTON 26
#define BLUE_BUTTON 19

typedef enum button_events button_event_t;

void button_init(unsigned pin);

int get_button(unsigned pin);

// rb_t* get_button_rb(unsigned pin);

button_event_t check_button_dequeue(unsigned pin);

