#include "../../include/button.h"
#include "gl.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "printf.h"
#include "strings.h"
#include "timer.h"
#include "uart.h"

#define BUTTON 21

const char *ca_types[] = {"Game of Life", "WireWorld"};

void draw_text_centered(unsigned y, const char *str, color_t c) {
  unsigned width = gl_get_width();
  unsigned x = (width / 2) - (gl_get_char_width() * strlen(str) / 2);
  gl_draw_string(x, y, str, c);
}

void main() {
  uart_init();
  timer_init();
  gpio_init();
  gpio_set_pullup(BUTTON);
  gpio_set_input(BUTTON);

  // setup interrupts
  interrupts_init();
  button_init(BUTTON);
  interrupts_global_enable();

  // start gl
  gl_init(1024, 576, GL_DOUBLEBUFFER);

  const unsigned num_presets = sizeof(ca_types) / sizeof(char *);

  unsigned choice = 0;
  while (true) {
    // draw menu
    gl_clear(GL_BLACK);
    draw_text_centered(5, "Choose Cellular Automata:", GL_WHITE);
    for (int i = 0; i < num_presets; i++) {
      if (i == choice) {
        draw_text_centered(gl_get_height() / 2 + i * gl_get_char_height(),
                           ca_types[i], GL_BLUE);
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
    choice = (choice + 1) % num_presets;
    printf("change choice\n");
  }
  printf("Detected hold\n");

  uart_send(EOT);
}
