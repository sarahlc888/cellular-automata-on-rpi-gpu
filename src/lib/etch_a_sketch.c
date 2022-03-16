/*
 * Avi Udash
 * 03/15/2022
 * Code for CS107E Final Project
 *
 * This module implements the etch-a-sketch functionality for the cellular
 * automata preset interface.
 */

#include "../../include/etch_a_sketch.h"
#include "../../include/button.h"
#include "../../include/mcp3008.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "uart.h"

/*
 * All the color_buttons used in this project.
 *
 * format: {button_pin, button_color, led_pin}
 */
const color_buttons_t color_buttons[] = {
    {GPIO_PIN20, GL_WHITE, GPIO_PIN12}, // white button
    {GPIO_PIN16, GL_BLACK, GPIO_PIN0},  // black button
    {GPIO_PIN26, GL_RED, GPIO_PIN6},    // red button
    {GPIO_PIN19, GL_BLUE, GPIO_PIN5}};  // blue button

// calc number of color buttons
const unsigned num_color_buttons =
    sizeof(color_buttons) / sizeof(color_buttons_t);

/*
 * Helper function that checks if the given 'color' is in the 'color_states'
 * array
 */
static bool color_in_states(color_t color, color_t color_states[],
                            size_t num_colors) {
  for (int i = 0; i < num_colors; i++) {
    if (color_states[i] == color) {
      return true;
    }
  }

  return false;
}

/*
 * Function: mcp3008_read
 * --------------------------
 *  @param color_t color_states[]: array of all possible color states in the CA
 *  @param size_t num_colors: number of color_states in the above array
 *  @param unsigned main_button: GPIO pin for the main button, so that the
 * holding the button can interrupt the etch-a-sketch and the CA runs
 * --------------------------
 *  The function reads data from the potentiometers and draws on the screen
 * until the main button is held down
 */
void etch_a_sketch(color_t color_states[], size_t num_colors,
                   unsigned main_button) {
  // store commonly accessed data in consts
  const unsigned gl_width = gl_get_width();
  const unsigned gl_height = gl_get_height();

  // default start color is the first non-background color
  color_t curr_color = color_states[1];

  // exponential smoothing to get smoother read from potentiometers
  int alpha = 30;
  int x = (mcp3008_read(CH0) * gl_width) / 1023;
  int y = (mcp3008_read(CH1) * gl_height) / 1023;

  // drawing loop
  while (true) {
    // get x and y values from potentiometers
    int new_x = (mcp3008_read(CH0) * gl_width) / 1023;
    int new_y = (mcp3008_read(CH1) * gl_height) / 1023;

    /*
     * exponential smoothing algorithm
     * src: https://en.wikipedia.org/wiki/Exponential_smoothing
     */
    x += (alpha * (new_x - x)) / 100;
    y += (alpha * (new_y - y)) / 100;

    // draw the pixel in both buffers
    gl_draw_pixel(x, y, curr_color);
    gl_swap_buffer();
    gl_draw_pixel(x, y, curr_color);

    // check if main button has detected a hold
    if (check_button_dequeue(main_button) == BUTTON_HOLD) {
      break;
    }

    // check all color buttons and change color if needed
    for (int i = 0; i < num_color_buttons; i++) {
      // check if button is pressed down and the color is in the color states
      if (!gpio_read(color_buttons[i].pin) &&
          color_in_states(color_buttons[i].color, color_states, num_colors)) {
        curr_color = color_buttons[i].color;
      }
    }

    // check which color led to turn on
    for (int i = 0; i < num_color_buttons; i++) {
      if (curr_color == color_buttons[i].color) {
        gpio_write(color_buttons[i].led_pin, 1); // turn on led for color
      } else {
        gpio_write(color_buttons[i].led_pin, 0); // else turn off
      }
    }
  }
}
