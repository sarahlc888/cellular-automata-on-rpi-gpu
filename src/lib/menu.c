/* Avi Udash
 * 03/09/2022
 * Code for CS107E Final Project
 *
 * This module supports displaying and interacting with the menus in the
 * program.
 *
 * The module defines menu items for the different menus in the project:
 * main menu, presets for game of life, presets for wireworld.
 *
 * Implements a draw_text_centered helper function to make the menu look
 * nicer. The main function is 'display_menu()' which creates an interactive
 * menu using the configured options from its parameters
 *
 */
#include "../../include/menu.h"
#include "../../include/button.h"
#include "gl.h"
#include "strings.h"
#include "uart.h"

/*
 * The main menu contains all the implemented cellular automata.
 * Preset_fn_t is NULL since all menu items lead to a new menu screen
 * for respective cellular automata.
 * */
const preset_t main_presets[] = {{"Game of Life", (preset_fn_t)NULL},
                                 {"WireWorld", (preset_fn_t)NULL}};

// Presets for the game of life
const preset_t life_presets[] = {
    {"Preset 1", (preset_fn_t)create_life_preset},
    {"Preset 2", (preset_fn_t)create_life_preset2},
    {"Custom", (preset_fn_t)create_custom_preset},
};

// Presets for WireWorld
const preset_t ww_presets[] = {
    {"Preset 1", (preset_fn_t)create_ww_preset},
    {"Custom", (preset_fn_t)create_custom_preset},
};

// calculate the number of presets for each of the menus
const unsigned num_main_presets = sizeof(main_presets) / sizeof(preset_t);
const unsigned num_life_presets = sizeof(life_presets) / sizeof(preset_t);
const unsigned num_ww_presets = sizeof(ww_presets) / sizeof(preset_t);

/*
 * Function: draw_text_centered
 * --------------------------
 * Helper function that horizontally centers the passed in 'str', starting on
 * 'y' value, with color 'c'
 *
 * @param unsigned 'y': y-value to draw text on
 * @param const char* 'str': string to draw
 * @param color_t 'c': text color
 */
void draw_text_centered(unsigned y, const char *str, color_t c) {
  unsigned width = gl_get_width();
  unsigned x = (width / 2) - (gl_get_char_width() * strlen(str) / 2);
  gl_draw_string(x, y, str, c);
}

/*
 * Function: display_menu
 * --------------------------
 * This function displays a menu on a display, using a button as user
 * input. The user is presented with all the possible menu options and pressing
 * the button will allow user to switch options. Holding the button confirms the
 * option and will exit the function. The function returns the user choice,
 * where 0 is the first menu item.
 *
 * @param const char* 'title': title of menu, will be drawn at top
 * @param preset_t presets[]: array of all menu options
 * @param size_t num_presets: number of menu options/presets
 * @param unsigned 'main_button': gpio pin of main button
 *
 * @return unsigned: user choice, from 0 to num_presets-1
 */
unsigned display_menu(const char *title, const preset_t presets[],
                      size_t num_presets, unsigned main_button) {
  // store user choice
  unsigned choice = 0;

  // main loop
  while (true) {
    // black background
    gl_clear(GL_BLACK);

    // draw title
    draw_text_centered(5, title, GL_WHITE);

    // draw all choices, BLUE for currently selected, WHITE otherwise
    for (int i = 0; i < num_presets; i++) {
      if (i == choice) {
        draw_text_centered(gl_get_height() / 2 + i * gl_get_char_height(),
                           presets[i].name, GL_BLUE); // selected choice
      } else {
        draw_text_centered(gl_get_height() / 2 + i * gl_get_char_height(),
                           presets[i].name, GL_WHITE);
      }
    }
    gl_swap_buffer();

    // check if button hold, and exit if so
    if (get_button(main_button) == BUTTON_HOLD) {
      break;
    }

    // increment choice
    choice = (choice + 1) % num_presets;
  }

  return choice;
}
