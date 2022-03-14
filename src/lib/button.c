#include "../../include/button.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "ringbuffer.h"
#include "timer.h"
#include "uart.h"

unsigned button_pin;
rb_t *rb;

static void handle_button(unsigned pc, void *aux_data) {

  gpio_clear_event(button_pin);

  unsigned start = timer_get_ticks();
  while (!gpio_read(button_pin))
    ;
  unsigned stop = timer_get_ticks();

  if (stop - start > 10000) {     // deal with debouncing
    if (stop - start > 1000000) { // hold time is 1s
      rb_enqueue(rb, BUTTON_HOLD);
    } else {
      rb_enqueue(rb, BUTTON_PRESS);
    }
  }
}

void button_init(unsigned pin) {
  button_pin = pin;

  rb = rb_new();

  gpio_set_pullup(pin);
  gpio_interrupts_init();
  gpio_enable_event_detection(pin, GPIO_DETECT_FALLING_EDGE);
  gpio_interrupts_register_handler(pin, handle_button, NULL);
  gpio_interrupts_enable();
}

int get_button(unsigned pin) {
  int event;
  while (rb_empty(rb))
    ;
  rb_dequeue(rb, &event);

  return event;
}

rb_t *get_button_rb(unsigned pin) { return rb; }
