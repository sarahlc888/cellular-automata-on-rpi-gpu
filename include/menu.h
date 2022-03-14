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
#ifndef MENU_H
#define MENU_H

#include "gl.h"
#include "../include/ca.h"
#include "strings.h"

/*
 * 'preset_t' is a struct that stores a single preset/menu option, with a name 
 * and a preset function, used to run in ca_run()
 */
typedef struct {
  const char *name; // name of preset
  preset_fn_t fn;   // preset function to run in ca
} preset_t;

// the presets are defined in the menu.c files
extern const preset_t main_presets[];
extern const preset_t life_presets[];
extern const preset_t ww_presets[];

// the number of presets for each menu, calculated in menu.c file
extern const unsigned num_main_presets;
extern const unsigned num_life_presets;
extern const unsigned num_ww_presets;

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
                  size_t num_presets, unsigned main_button);

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
void draw_text_centered(unsigned y, const char *str, color_t c);

#endif
