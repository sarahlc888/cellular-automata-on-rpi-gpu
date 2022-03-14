#include "../../include/menu.h"
#include "../../include/button.h"
#include "gl.h"
#include "strings.h"
#include "uart.h"

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

const unsigned num_main_presets = sizeof(main_presets) / sizeof(preset_t);
const unsigned num_life_presets = sizeof(life_presets) / sizeof(preset_t);
const unsigned num_ww_presets = sizeof(ww_presets) / sizeof(preset_t);

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
