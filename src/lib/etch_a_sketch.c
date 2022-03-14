#include "../../include/etch_a_sketch.h"
#include "../../include/button.h"
#include "../../include/mcp3008.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "uart.h"

// typedef struct {
//   unsigned pin;     // gpio pin of button
//   color_t color;    // button color
//   unsigned led_pin; // gpio pin of corresponding led
// } color_buttons_t;

color_buttons_t color_buttons[] = {{WHITE_BUTTON, GL_WHITE, 12},
                                   {BLACK_BUTTON, GL_BLACK, 00},
                                   {RED_BUTTON, GL_RED, 06},
                                   {BLUE_BUTTON, GL_BLUE, 05}};

const unsigned num_color_buttons =
    sizeof(color_buttons) / sizeof(color_buttons_t);

static bool color_in_states(color_t color, color_t color_states[],
                            size_t num_colors) {
  for (int i = 0; i < num_colors; i++) {
    if (color_states[i] == color) {
      return true;
    }
  }

  printf("false");
  return false;
}

void etch_a_sketch(color_t color_states[], size_t num_colors) {
  // store ringbuffer for button
  // rb_t *rb = get_button_rb(MAIN_BUTTON);

  color_t curr_color = color_states[1];

  gl_init(100, 100, GL_DOUBLEBUFFER);

  // init gl with background color
  gl_clear(color_states[0]);
  gl_swap_buffer();
  gl_clear(color_states[0]);
  gl_swap_buffer();

  printf("clear gl\n");

  // drawing loop
  while (true) {
    // get x and y values from potentiometers
    int x = (mcp3008_read(CH0) * gl_get_width()) / 900;
    int y = (mcp3008_read(CH1) * gl_get_height()) / 900;

    // draw the pixel in both buffers
    gl_draw_pixel(x, y, curr_color);
    gl_swap_buffer();
    gl_draw_pixel(x, y, curr_color);

    // check if main button has detected a hold
    if (check_button_dequeue(MAIN_BUTTON) == BUTTON_HOLD) {
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
