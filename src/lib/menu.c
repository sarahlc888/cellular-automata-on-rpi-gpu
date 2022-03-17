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
 *
 * Options don't matter, since each just lead into a sub-menu
 * */
const preset_t main_presets[] = {{"Game of Life",
                                  (preset_fn_t)NULL,
                                  {.width = 1024,
                                   .height = 1024,
                                   .delay_ms = 0,
                                   .run_time = 1000000 * 100,
                                   .use_time_limit = true,
                                   .preset_file = "",
                                   .save_preset = false}},

                                 {"WireWorld",
                                  (preset_fn_t)NULL,
                                  {.width = 1024,
                                   .height = 1024,
                                   .delay_ms = 0,
                                   .run_time = 1000000 * 100,
                                   .use_time_limit = true,
                                   .preset_file = "",
                                   .save_preset = false}}};

// Presets for the game of life
const preset_t life_presets[] = {{"Simple Oscillation",
                                  (preset_fn_t)create_life_blinker_preset,
                                  {.width = 256,
                                   .height = 256,
                                   .delay_ms = 300,
                                   .run_time = 1000000 * 100,
                                   .use_time_limit = true,
                                   .preset_file = "",
                                   .save_preset = false}},

                                 {"Random Start",
                                  (preset_fn_t)create_random_life_preset,
                                  {.width = 1024,
                                   .height = 1024,
                                   .delay_ms = 50,
                                   .run_time = 1000000 * 100,
                                   .use_time_limit = true,
                                   .preset_file = "",
                                   .save_preset = false}},

                                 {"Bunnies",
                                  (preset_fn_t)create_life_bunnies,
                                  {.width = 128,
                                   .height = 128,
                                   .delay_ms = 0,
                                   .run_time = 1000000 * 100,
                                   .use_time_limit = false,
                                   .preset_file = "",
                                   .save_preset = false}},

                                 {"Mandalas",
                                  (preset_fn_t)create_life_karel_177,
                                  {.width = 128,
                                   .height = 128,
                                   .delay_ms = 50,
                                   .run_time = 1000000 * 100,
                                   .use_time_limit = false,
                                   .preset_file = "",
                                   .save_preset = false}},

                                 {"Flying Wing",
                                  (preset_fn_t)create_life_flying_wing,
                                  {.width = 512,
                                   .height = 512,
                                   .delay_ms = 0,
                                   .run_time = 1000000 * 100,
                                   .use_time_limit = false,
                                   .preset_file = "",
                                   .save_preset = false}},

                                 {"Custom",
                                  (preset_fn_t)create_custom_preset,
                                  {.width = 128,
                                   .height = 128,
                                   .delay_ms = 300,
                                   .run_time = 1000000 * 100,
                                   .use_time_limit = true,
                                   .preset_file = "",
                                   .save_preset = false}}};

// Presets for WireWorld
const preset_t ww_presets[] = {{"OR, XOR, AND",
                                (preset_fn_t)create_ww_preset,
                                {.width = 64,
                                 .height = 64,
                                 .delay_ms = 300,
                                 .run_time = 1000000 * 100,
                                 .use_time_limit = true,
                                 .preset_file = "",
                                 .save_preset = false}},

                               {"AND Gates",
                                (preset_fn_t)create_ww_AND_gates,
                                {.width = 64,
                                 .height = 64,
                                 .delay_ms = 300,
                                 .run_time = 1000000 * 100,
                                 .use_time_limit = true,
                                 .preset_file = "",
                                 .save_preset = false}},

                               {"Nyles Heise Multiplication",
                                (preset_fn_t)create_ww_nh_multiplication,
                                {.width = 208,
                                 .height = 208,
                                 .delay_ms = 0,
                                 .run_time = 1000000 * 100,
                                 .use_time_limit = true,
                                 .preset_file = "",
                                 .save_preset = false}},

                               {"Custom",
                                (preset_fn_t)create_custom_preset,
                                {.width = 64,
                                 .height = 64,
                                 .delay_ms = 300,
                                 .run_time = 1000000 * 100,
                                 .use_time_limit = true,
                                 .preset_file = "",
                                 .save_preset = false}}};

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

  // colors
  color_t BG_COLOR = gl_color(0x1d, 0x1e, 0x2c);
  color_t SELECT_COLOR = gl_color(0xf7, 0x5c, 0x03);
  color_t FG_COLOR = gl_color(0xff, 0xff, 0xff);
  color_t SECOND_COLOR = gl_color(0x17, 0x68, 0xac);

  // main loop
  while (true) {
    // draw background
    gl_clear(BG_COLOR);

    // draw title box
    gl_draw_rect(0, 0, gl_get_width(), 40 + gl_get_char_height(), SECOND_COLOR);

    // draw title
    draw_text_centered(20, title, FG_COLOR);

    // variables to help draw
    unsigned text_y =
        gl_get_width() / 2 - num_presets * gl_get_char_height() / 2;

    // draw all choices, SELECT_COLOR for currently selected, FG_COLOR otherwise
    for (int i = 0; i < num_presets; i++) {

      if (i == choice) {
        unsigned box_x1 =
            gl_get_width() / 2 -
            (gl_get_char_width() * (strlen(presets[i].name) + 2)) / 2;
        unsigned box_y1 = text_y - 5;
        unsigned box_w = gl_get_char_width() * (strlen(presets[i].name) + 2);
        unsigned box_h = gl_get_char_height() + 10;

        gl_draw_rect(box_x1, box_y1, box_w, box_h, SELECT_COLOR);
      }

      draw_text_centered(text_y, presets[i].name, FG_COLOR); // selected choice

      text_y += gl_get_char_height() + 15;
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
