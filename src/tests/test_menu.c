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

typedef struct {
  unsigned pin;     // gpio pin of button
  color_t color;    // button color
  unsigned led_pin; // gpio pin of corresponding led
} color_buttons_t;

color_buttons_t color_buttons[] = {{WHITE_BUTTON, GL_WHITE, 12},
                                   {BLACK_BUTTON, GL_BLACK, 00},
                                   {RED_BUTTON, GL_RED, 06},
                                   {BLUE_BUTTON, GL_BLUE, 05}};

typedef struct {
  const char *name; // name of preset
  preset_fn_t fn;   // preset function to run in ca
} preset_t;

const char *ca_types[] = {"Game of Life", "WireWorld"};

preset_t main_presets[] = {{"Game of Life", (preset_fn_t)NULL},
                           {"WireWorld", (preset_fn_t)NULL}};

preset_t life_presets[] = {
    {"Preset 1", (preset_fn_t)create_life_preset},
    {"Preset 2", (preset_fn_t)create_life_preset2},
    {"Custom", (preset_fn_t)create_custom_preset},
};

preset_t ww_presets[] = {
    {"Preset 1", (preset_fn_t)create_ww_preset},
    {"Custom", (preset_fn_t)create_custom_preset},
};

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

void draw_text_centered(unsigned y, const char *str, color_t c) {
  unsigned width = gl_get_width();
  unsigned x = (width / 2) - (gl_get_char_width() * strlen(str) / 2);
  gl_draw_string(x, y, str, c);
}

void display_menu(unsigned *choice, preset_t presets[], const char *title,
                  size_t arr_size) {

  while (true) {
    // draw menu
    gl_clear(GL_BLACK);
    draw_text_centered(5, title, GL_WHITE);

    // draw choices
    for (int i = 0; i < arr_size; i++) {
      if (i == *choice) {
        draw_text_centered(gl_get_height() / 2 + i * gl_get_char_height(),
                           presets[i].name, GL_BLUE); // selected choice
      } else {
        draw_text_centered(gl_get_height() / 2 + i * gl_get_char_height(),
                           presets[i].name, GL_WHITE);
      }
    }
    gl_swap_buffer();

    // check if button press or hold
    if (get_button(MAIN_BUTTON) == BUTTON_HOLD) {
      break;
    }

    // increment choice
    *choice = (*choice + 1) % arr_size;
  }
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
    for (int i = 0; i < sizeof(color_buttons) / sizeof(color_buttons_t); i++) {
      // check if button is pressed down and the color is in the color states
      if (!gpio_read(color_buttons[i].pin) &&
          color_in_states(color_buttons[i].color, color_states, num_colors)) {
        curr_color = color_buttons[i].color;
      }
    }

    // check which color led to turn on
    for (int i = 0; i < sizeof(color_buttons) / sizeof(color_buttons_t); i++) {
      if (curr_color == color_buttons[i].color) {
        gpio_write(color_buttons[i].led_pin, 1); // turn on led for color
      } else {
        gpio_write(color_buttons[i].led_pin, 0); // else turn off
      }
    }
  }
}

void main() {
  uart_init();
  timer_init();
  gpio_init();
  mcp3008_init();

  // setup main button
  gpio_set_pullup(MAIN_BUTTON);
  gpio_set_input(MAIN_BUTTON);

  // setup color buttons
  for (int i = 0; i < sizeof(color_buttons) / sizeof(color_buttons_t); i++) {
    gpio_set_pullup(color_buttons[i].pin);
    gpio_set_input(color_buttons[i].pin);
    gpio_set_pulldown(color_buttons[i].led_pin);
    gpio_set_output(color_buttons[i].led_pin);
  }

  // setup interrupts
  interrupts_init();
  button_init(MAIN_BUTTON);
  interrupts_global_enable();

  // start gl
  gl_init(512, 512, GL_DOUBLEBUFFER);

  // menu
  unsigned choice = 0;
  display_menu(&choice, main_presets, "Choose Cellular Automata:",
               sizeof(main_presets) / sizeof(preset_t));

  // variables for ca
  unsigned save_preset = 0;
  unsigned use_time_limit = 1;

  unsigned int delay_ms = 300;
  unsigned int run_time = 1000000 * 100;
  const char *preset_file = "/presets/life_50x50_1.rgba";

  // Run the game corresponding to the choice
  if (choice == LIFE) {

    // reset choice to 0
    choice = 0;
    unsigned num_presets = sizeof(life_presets) / sizeof(preset_t);

    // display life menu
    display_menu(&choice, life_presets,
                 "Choose Game of Life Preset:", num_presets);

    // init colors
    color_t color_states[2] = {GL_BLACK, GL_WHITE};
    gl_init(100, 100, GL_DOUBLEBUFFER);

    // game of life
    ca_init(LIFE, gl_get_width(), gl_get_height(), color_states, delay_ms);

    // if custom (last_choice)
    if (choice == num_presets - 1) {
      etch_a_sketch(color_states, 2);
    } else {
      ca_create_and_load_preset(preset_file, life_presets[choice].fn,
                                save_preset);
    }

    ca_run(use_time_limit, run_time);

  } else if (choice == WIREWORLD) {
    // reset choice to 0
    choice = 0;
    unsigned num_presets = sizeof(ww_presets) / sizeof(preset_t);

    // display wireworld menu
    display_menu(&choice, ww_presets, "Choose WireWorld Preset:", num_presets);

    // init gl and colors
    gl_init(100, 100, GL_DOUBLEBUFFER);
    color_t color_states[4] = {GL_WHITE, GL_BLUE, GL_RED, GL_BLACK};

    // init the ca
    ca_init(WIREWORLD, gl_get_width(), gl_get_height(), color_states, delay_ms);

    // if custom (last choice)
    if (choice == num_presets - 1) {
      etch_a_sketch(color_states, 4);
    } else {
      ca_create_and_load_preset(preset_file, ww_presets[choice].fn,
                                save_preset);
    }

    // run the ca
    ca_run(use_time_limit, run_time);
  }

  uart_send(EOT);
}
