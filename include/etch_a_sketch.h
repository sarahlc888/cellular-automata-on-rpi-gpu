#ifndef ETCH_A_SKETCH_H
#define ETCH_A_SKETCH_H

#include "../include/gl.h"
#include "../include/button.h"

typedef struct {
  unsigned pin;     // gpio pin of button
  color_t color;    // button color
  unsigned led_pin; // gpio pin of corresponding led
} color_buttons_t;

extern const color_buttons_t color_buttons[];
extern const unsigned num_color_buttons;

void etch_a_sketch(color_t color_states[], size_t num_colors, unsigned main_button);

#endif
