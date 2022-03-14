/* Avi Udash
 * 03/09/2022
 * Code for CS107E Final Project
 *
 * This module supports initializing button interrupts and getting button events
 *
 * TODO:
 * The module also supports handling interrupts for multiple buttons in one
 * program. It stores an array of rb_t's, one for each gpio pin. When a button
 * is initialized, it creates a ringbuffer for that specific button, which can
 * later be enqueued to or dequeued from.
 * TODO:
 * Currently, the modules supports handling interrupts for one button at a time.
 * The module uses a ring buffer to keep track of button presses, which can then
 * be deqeued at a later time.
 */
#include "../../include/button.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "malloc.h"
#include "ringbuffer.h"
#include "timer.h"
#include "uart.h"

// array of all ringbuffer pointers, one for each gpio pin
static rb_t *rb_arr[GPIO_PIN_LAST];

/*
 * Function: handle_button
 * --------------------------
 * This function is called whenever the system detects a falling edge on the
 * button gpio pin. It checks how long the button is held for and enqeues
 * either a button hold or a button press depending on the length of the press.
 *
 * TODO:
 * The 'aux_data' field expects a pointer to a ringbuffer (basically rb_t) for
 * the pin in which the falling edge was detected.
 *
 * The 'aux_data' field expects a pointer to an unsigned int representing the
 * gpio pin number for the button that was detected
 */
static void handle_button(unsigned pc, void *aux_data) {
  unsigned button_pin = *(unsigned *)aux_data;

  // make sure to clear the event
  gpio_clear_event(button_pin);

  // start timer until button is released
  unsigned start = timer_get_ticks();
  while (!gpio_read(button_pin))
    ;
  unsigned stop = timer_get_ticks();

  // check time
  if (stop - start > 10000) {     // deal with debouncing
    if (stop - start > 1000000) { // hold time is 1s
      rb_enqueue(rb_arr[button_pin], BUTTON_HOLD);
      uart_putchar('1');
    } else {
      rb_enqueue(rb_arr[button_pin], BUTTON_PRESS);
      uart_putchar('0');
    }
  }
}

/*
 * Function: button_init
 * --------------------------
 * Initialize the button on the given gpio 'pin'.
 * Create and store a new rb into the array of ringbuffers.
 * Enable gpio interrupts and register handler function for falling edge.
 */
void button_init(unsigned pin) {
  rb_arr[pin] = rb_new();

  // need to malloc pin number so we can send to handler as aux_data
  unsigned *pin_ptr = malloc(sizeof(unsigned));
  *pin_ptr = pin;

  gpio_set_pullup(pin);
  gpio_interrupts_init();
  gpio_enable_event_detection(pin, GPIO_DETECT_FALLING_EDGE);
  gpio_interrupts_register_handler(pin, handle_button, pin_ptr);
  gpio_interrupts_enable();
}

/*
 * Function: get_button
 * --------------------------
 * This function waits for until the rb for the specified pin contains a
 * button_event and then returns the event
 */
button_event_t get_button(unsigned pin) {
  int event;
  while (rb_empty(rb_arr[pin]))
    ;
  rb_dequeue(rb_arr[pin], &event);

  return event;
}

/*
 * Function: check_button_dequeue
 * --------------------------
 * This function checks if the ringbuffer for the pin contains an event.
 * If so, it returns the most recent event.
 * If empty, it return BUTTON_EMPTY event.
 */
button_event_t check_button_dequeue(unsigned pin) {
  // if rb is empty, just return empty button
  if (rb_empty(rb_arr[pin])) {
    return BUTTON_EMPTY;
  }

  int event;
  rb_dequeue(rb_arr[pin], &event);
  return event;
}
