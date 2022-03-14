#include "../../include/button.h"
#include "../../include/ca.h"
#include "../../include/mcp3008.h"
#include "gl.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "printf.h"
#include "ringbuffer.h"
#include "strings.h"
#include "timer.h"
#include "uart.h"

#define BUTTON 21

#define WHITE_BUTTON 20
#define BLACK_BUTTON 16
#define RED_BUTTON 26
#define YELLOW_BUTTON 19

const char *ca_types[] = {"Game of Life", "WireWorld", "Custom Game of Life",
                          "Custom Wireworld"};

void draw_text_centered(unsigned y, const char *str, color_t c) {
  unsigned width = gl_get_width();
  unsigned x = (width / 2) - (gl_get_char_width() * strlen(str) / 2);
  gl_draw_string(x, y, str, c);
}

void display_menu(unsigned *choice) {
  // menu
  while (true) {
    // draw menu
    gl_clear(GL_BLACK);
    draw_text_centered(5, "Choose Cellular Automata:", GL_WHITE);

    // draw choices
    const unsigned num_presets = sizeof(ca_types) / sizeof(char *);
    for (int i = 0; i < num_presets; i++) {
      if (i == *choice) {
        draw_text_centered(gl_get_height() / 2 + i * gl_get_char_height(),
                           ca_types[i], GL_BLUE); // selected choice
      } else {
        draw_text_centered(gl_get_height() / 2 + i * gl_get_char_height(),
                           ca_types[i], GL_WHITE);
      }
    }
    gl_swap_buffer();

    // check if button press or hold
    if (get_button(BUTTON) == BUTTON_HOLD) {
      break;
    }

    // increment choice
    *choice = (*choice + 1) % num_presets;
  }
};

void main() {
  uart_init();
  timer_init();
  gpio_init();
  mcp3008_init();
  gpio_set_pullup(BUTTON);
  gpio_set_input(BUTTON);

  // setup interrupts
  interrupts_init();
  button_init(BUTTON);
  interrupts_global_enable();

  // start gl
  gl_init(512, 512, GL_DOUBLEBUFFER);

  unsigned choice = 0;

  display_menu(&choice);

  // variables for ca
  unsigned save_preset = 0;
  unsigned use_time_limit = 1;

  // Run the game corresponding to the choice
  if (choice == LIFE) {

    unsigned int delay_ms = 1000;
    unsigned int run_time = 1000000 * 10;
    const char *preset_file = "/presets/life_50x50_1.rgba";

    // game of life
    color_t color_states[2] = {GL_BLACK, GL_WHITE};
    gl_init(50, 50, GL_DOUBLEBUFFER);
    ca_init(LIFE, 50, 50, color_states, delay_ms);
    // ca_init(LIFE, 1000, 1000, color_states, 1000)

    ca_create_and_load_preset(preset_file, (preset_fn_t)create_life_preset2,
                              save_preset);
    ca_run(use_time_limit, run_time);

  } else if (choice == WIREWORLD) {

    unsigned int delay_ms = 100;
    unsigned int run_time = 1000000 * 100;
    const char *preset_file = "/presets/life_50x50_1.rgba";
    color_t color_states[4] = {GL_WHITE, GL_YELLOW, GL_RED, GL_BLACK};

    // run wireworld
    gl_init(100, 100, GL_DOUBLEBUFFER);
    ca_init(WIREWORLD, 100, 100, color_states, delay_ms);
    ca_create_and_load_preset(preset_file, (preset_fn_t)create_ww_preset,
                              save_preset);
    ca_run(use_time_limit, run_time);

  } else if (choice == CUSTOM_LIFE) {

    // store ringbuffer for button
    rb_t *rb = get_button_rb(BUTTON);
    bool is_drawing = true;

    gl_init(100, 100, GL_DOUBLEBUFFER);

    gl_clear(GL_BLACK);
    gl_swap_buffer();
    gl_clear(GL_BLACK);

    while (true) {
      int x = (mcp3008_read(CH0) * gl_get_width()) / 1000;
      int y = (mcp3008_read(CH1) * gl_get_height()) / 1000;
      // printf("x: %d, y: %d\n", x, y);
      if (is_drawing) {
        gl_draw_pixel(x, y, GL_WHITE);
        gl_swap_buffer();
        gl_draw_pixel(x, y, GL_WHITE);
      }

      if (!rb_empty(rb)) {
        int event;
        rb_dequeue(rb, &event);
        if (event == BUTTON_PRESS) {
          is_drawing = !is_drawing;
        } else {
          break;
        }
      }
    }

    // run game of life
    color_t color_states[2] = {GL_BLACK, GL_WHITE};
    unsigned int delay_ms = 100;
    unsigned int run_time = 1000000 * 1000;
    ca_init(LIFE, gl_get_width(), gl_get_height(), color_states, delay_ms);
    ca_run(use_time_limit, run_time);
  } else if (choice == CUSTOM_WIREWORLD) {

    // store ringbuffer for button
    rb_t *rb = get_button_rb(BUTTON);

    // init colors
    color_t color_states[4] = {GL_WHITE, GL_YELLOW, GL_RED, GL_BLACK};
    unsigned num_colors = 4;
    unsigned curr_color = 3;

    gl_init(20, 20, GL_DOUBLEBUFFER);

    // init gl with background color
    gl_clear(color_states[0]);
    gl_swap_buffer();
    gl_clear(color_states[0]);
    gl_swap_buffer();

    // drawing loop
    while (true) {
      // get x and y values from potentiometers
      int x = (mcp3008_read(CH0) * gl_get_width()) / 900;
      int y = (mcp3008_read(CH1) * gl_get_height()) / 900;

      // draw the pixel in both buffers
      gl_draw_pixel(x, y, color_states[curr_color]);
      gl_swap_buffer();
      gl_draw_pixel(x, y, color_states[curr_color]);

      // check if button has been pressed
      if (!rb_empty(rb)) {
        int event;
        rb_dequeue(rb, &event);
        if (event == BUTTON_PRESS) {
          curr_color = (curr_color + 1) % num_colors;
        } else {
          break;
        }
      }
    }

    // run wireworld
    unsigned int delay_ms = 1000;
    unsigned int run_time = 1000000 * 100;
    ca_init(WIREWORLD, gl_get_width(), gl_get_height(), color_states, delay_ms);
    ca_run(use_time_limit, run_time);
  }

  printf("chose %s\n", ca_types[choice]);

  uart_send(EOT);
}
