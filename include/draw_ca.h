#ifndef DRAWCA_H
#define DRAWCA_H

#include "gl.h"

// specify gate
typedef enum {
    WW_OR = 0,
    WW_AND = 1,
    WW_XOR = 2
} gate_ind_t;

// for wire world
typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int in_row_1;
    unsigned int in_row_2;
    unsigned int out_row;
    unsigned int gate_data[];
} gate_t;

// pattern for Life
typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int data[];
} pattern_t;

// type of function pointer used to create a preset
typedef unsigned int (*preset_fn_t)(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

void create_life_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);
void create_ww_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);
void create_life_preset2(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);
void create_random_life_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);
void create_custom_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);


#endif
