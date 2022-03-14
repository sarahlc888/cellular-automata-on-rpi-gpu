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
#ifndef BUTTON_H
#define BUTTON_H

#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "ringbuffer.h"


// The 'button_event_t' defines values that represent different button events
typedef enum button_events{
  BUTTON_PRESS = 0, // less than 1 sec
  BUTTON_HOLD = 1, // greater than  1 sec
  BUTTON_EMPTY = -1 // detected no press
} button_event_t;


/*
 * Function: button_init
 * --------------------------
 * Initialize the button on the given gpio 'pin'.
 * Create and store a new rb into the array of ringbuffers.
 * Enable gpio interrupts and register handler function for falling edge.
 */
void button_init(unsigned pin);

/*
 * Function: get_button
 * --------------------------
 * This function waits for until the rb for the specified pin contains a
 * button_event and then returns the event
 */
button_event_t get_button(unsigned pin);

/*
 * Function: check_button_dequeue
 * --------------------------
 * This function checks if the ringbuffer for the pin contains an event.
 * If so, it returns the most recent event.
 * If empty, it return BUTTON_EMPTY event.
 */
button_event_t check_button_dequeue(unsigned pin);

#endif
