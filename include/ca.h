#ifndef CA_H
#define CA_H

#include "gl.h"

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

void ca_init(unsigned int ca_mode, 
    unsigned int screen_width, unsigned int screen_height, int num_states,
    color_t* colors,
    unsigned int update_delay);

void ca_run(void);

#endif
