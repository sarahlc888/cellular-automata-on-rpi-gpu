/* Avi Udash
 * 03/09/2022
 * Code for CS107E Final Project
 *
 * Module to support etch-a-sketch functionality
 *
 */
#ifndef ETCH_A_SKETCH_H
#define ETCH_A_SKETCH_H

#include "../include/gl.h"
#include "../include/button.h"

// stores the color buttons
typedef struct {
  unsigned pin;     // gpio pin of button
  color_t color;    // button color
  unsigned led_pin; // gpio pin of corresponding led
} color_buttons_t;

// color buttons and num_color_buttons is defined in 'etch_a_sketch.c' file
extern const color_buttons_t color_buttons[];
extern const unsigned num_color_buttons;

/*
 * Function: mcp3008_read
 * --------------------------
 *  @param color_t color_states[]: array of all possible color states in the CA
 *  @param size_t num_colors: number of color_states in the above array
 *  @param unsigned main_button: GPIO pin for the main button, so that the holding the 
 *         button can interrupt the etch-a-sketch and the CA runs
 */
void etch_a_sketch(color_t color_states[], size_t num_colors, unsigned main_button);

#endif
