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

#define MAIN_BUTTON 21

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

    // main menu and cellular automata choice
    unsigned choice = display_menu("Choose Cellular Automata:", main_presets,
                                   num_main_presets, MAIN_BUTTON);

    // Run the game corresponding to the choice
    if (choice == LIFE) {

      // display life menu and get choice and options
      choice = display_menu("Choose Game of Life Preset:", life_presets,
                            num_life_presets, MAIN_BUTTON);

      preset_option_t options = life_presets[choice].options;

      // init colors
      color_t BG_COLOR = gl_color(0x1d, 0x1e, 0x2c);
      color_t color_states[2] = {BG_COLOR, GL_WHITE};

      // game of life
      ca_init(LIFE, options.width, options.height, color_states,
              options.delay_ms);

      // if custom (last_choice)
      if (choice == num_life_presets - 1) {
        etch_a_sketch(color_states, 2, MAIN_BUTTON);
      } else {
        ca_create_and_load_preset(options.preset_file, life_presets[choice].fn,
                                  options.save_preset);
      }

      ca_run(options.use_time_limit, options.run_time, MAIN_BUTTON);

    } else if (choice == WIREWORLD) {
      // display wireworld menu and get choice and options
      choice = display_menu("Choose WireWorld Preset:", ww_presets,
                            num_ww_presets, MAIN_BUTTON);
      preset_option_t options = ww_presets[choice].options;

      // init colors
      color_t BG_COLOR = gl_color(0x1d, 0x1e, 0x2c);
      color_t BLUE = gl_color(0x17, 0x68, 0xff);
      color_t ORANGE = gl_color(0xf7, 0x5c, 0x03);
      color_t color_states[4] = {BG_COLOR, ORANGE, BLUE, GL_WHITE};

      // init the ca
      ca_init(WIREWORLD, options.width, options.height, color_states,
              options.delay_ms);

      // if custom (last choice)
      if (choice == num_ww_presets - 1) {
        etch_a_sketch(color_states, 4, MAIN_BUTTON);
      } else {
        ca_create_and_load_preset(options.preset_file, ww_presets[choice].fn,
                                  options.save_preset);
      }

      // run the ca
      ca_run(options.use_time_limit, options.run_time, MAIN_BUTTON);
    }
  }

  uart_send(EOT);
}
