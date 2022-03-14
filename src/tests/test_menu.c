#include "../../include/button.h"
#include "../../include/ca.h"
#include "../../include/etch_a_sketch.h"
#include "../../include/mcp3008.h"
#include "../../include/menu.h"
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

// typedef struct {
//   const char *name; // name of preset
//   preset_fn_t fn;   // preset function to run in ca
// } preset_t;

// preset_t main_presets[] = {{"Game of Life", (preset_fn_t)NULL},
//                            {"WireWorld", (preset_fn_t)NULL}};

// preset_t life_presets[] = {
//     {"Preset 1", (preset_fn_t)create_life_preset},
//     {"Preset 2", (preset_fn_t)create_life_preset2},
//     {"Custom", (preset_fn_t)create_custom_preset},
// };
//
// preset_t ww_presets[] = {
//     {"Preset 1", (preset_fn_t)create_ww_preset},
//     {"Custom", (preset_fn_t)create_custom_preset},
// };

void main() {
  uart_init();
  timer_init();
  gpio_init();
  mcp3008_init();

  // setup main button
  gpio_set_pullup(MAIN_BUTTON);
  gpio_set_input(MAIN_BUTTON);

  // setup color buttons
  for (int i = 0; i < num_color_buttons; i++) {
    gpio_set_pullup(color_buttons[i].pin);
    gpio_set_input(color_buttons[i].pin);
    gpio_set_pulldown(color_buttons[i].led_pin);
    gpio_set_output(color_buttons[i].led_pin);
  }

  // setup interrupts
  interrupts_init();
  button_init(MAIN_BUTTON);
  interrupts_global_enable();

  // main program loop
  while (true) {
    // start gl
    gl_init(512, 512, GL_DOUBLEBUFFER);

    // menu
    unsigned choice = 0;
    display_menu(&choice, main_presets,
                 "Choose Cellular Automata:", num_main_presets);

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

      // display life menu
      display_menu(&choice, life_presets,
                   "Choose Game of Life Preset:", num_life_presets);

      // init colors
      color_t color_states[2] = {GL_BLACK, GL_WHITE};
      gl_init(100, 100, GL_DOUBLEBUFFER);

      // game of life
      ca_init(LIFE, gl_get_width(), gl_get_height(), color_states, delay_ms);

      // if custom (last_choice)
      if (choice == num_life_presets - 1) {
        etch_a_sketch(color_states, 2);
      } else {
        ca_create_and_load_preset(preset_file, life_presets[choice].fn,
                                  save_preset);
      }

      ca_run(use_time_limit, run_time);

    } else if (choice == WIREWORLD) {
      // reset choice to 0
      choice = 0;

      // display wireworld menu
      display_menu(&choice, ww_presets,
                   "Choose WireWorld Preset:", num_ww_presets);

      // init gl and colors
      gl_init(100, 100, GL_DOUBLEBUFFER);
      color_t color_states[4] = {GL_WHITE, GL_BLUE, GL_RED, GL_BLACK};

      // init the ca
      ca_init(WIREWORLD, gl_get_width(), gl_get_height(), color_states,
              delay_ms);

      // if custom (last choice)
      if (choice == num_ww_presets - 1) {
        etch_a_sketch(color_states, 4);
      } else {
        ca_create_and_load_preset(preset_file, ww_presets[choice].fn,
                                  save_preset);
      }

      // run the ca
      ca_run(use_time_limit, run_time);
    }
  }

  uart_send(EOT);
}
