#ifndef MENU_H
#define MENU_H

#include "gl.h"
#include "../include/ca.h"
#include "strings.h"

typedef struct {
  const char *name; // name of preset
  preset_fn_t fn;   // preset function to run in ca
} preset_t;

extern preset_t main_presets[];
extern preset_t life_presets[];
extern preset_t ww_presets[];

extern const unsigned num_main_presets;
extern const unsigned num_life_presets;
extern const unsigned num_ww_presets;

void display_menu(unsigned *choice, preset_t presets[], const char *title,
                  size_t arr_size);

void draw_text_centered(unsigned y, const char *str, color_t c);

#endif
