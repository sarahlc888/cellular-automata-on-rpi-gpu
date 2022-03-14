#ifndef CA_H
#define CA_H

#include "gl.h"
#include "draw_ca.h"

// cellular automata modes supported
typedef enum { LIFE = 0, WIREWORLD = 1, CUSTOM_LIFE = 2, CUSTOM_WIREWORLD = 3} ca_mode_t;

/*
 * This typedef gives a nickname to the type of function pointer used as the
 * automata simulator.  A ca_fn_t function has two parameters, the array
 * of tokens and its count. The return value is of type int.
 */
typedef unsigned int (*ca_fn_t)(unsigned int r, unsigned int c, void *state);

/*
 * This typedef defines the type for each entry in the cellular automata table.
 * A ca_option_t stores the info for one type of automata, including one-word name,
 * help description, and function pointer to run the automata.
 */
typedef struct _ca_option_struct {
    const char *name;
    const char *description;
    ca_fn_t fn;
} ca_option_t;

void ca_init(ca_mode_t ca_mode, 
    unsigned int screen_width, unsigned int screen_height, 
    color_t* colors,
    unsigned int update_delay);

void ca_create_and_load_preset(const char* fname, preset_fn_t make_preset, unsigned int save_to_sd);

void ca_run(unsigned int use_time_limit, unsigned int ticks_to_run, unsigned int main_button);

#endif
