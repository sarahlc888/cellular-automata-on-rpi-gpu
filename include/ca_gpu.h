/* Sarah Chen
 * 03/15/2022
 * Code for CS107E Final Project
 *
 * This module supports displaying cellular automata. The main steps are 
 * initializing the automaton, possibly creating and loading a preset, and 
 * running the simulation.
 */

#ifndef CAGPU_H
#define CAGPU_H

#include "gl.h"
#include "draw_ca.h"

// cellular automata modes supported
typedef enum { LIFE = 0, WIREWORLD = 1, CUSTOM_LIFE = 2, CUSTOM_WIREWORLD = 3} ca_mode_t;

/*
 * This typedef gives a nickname to the type of function pointer used as the
 * automata simulator.  A ca_fn_t function has two parameters, the array
 * of tokens and its count. The return value is of type int.
 */
typedef void (*ca_fn_t)(void *prev, void* next);

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

/*
 * Function: ca_init
 * --------------------------
 * Initialize the cellular automata simulation.
 */
void ca_init(ca_mode_t ca_mode, 
    unsigned int screen_width, unsigned int screen_height, 
    color_t* colors,
    unsigned int update_delay);

/*
 * Function: ca_create_and_load_preset
 * --------------------------
 * Create a preset and load it into the frame buffer. save it to the SD card if `save_to_sd` == 1.
 */
void ca_create_and_load_preset(const char* fname, preset_fn_t make_preset, unsigned int save_to_sd);

/*
 * Function: ca_run
 * --------------------------
 * Run the cellular automata simulation.
 */
void ca_run(unsigned int use_time_limit, unsigned int ticks_to_run, unsigned main_button, unsigned int verbose);

#endif
