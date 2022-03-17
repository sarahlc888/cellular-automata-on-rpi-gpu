/* Sarah Chen
 * 03/15/2022
 * Code for CS107E Final Project
 *
 * This module supports various structs and functions for drawing cellular
 * automata preset states, whether from specific pattern buidling blocks
 * or from pre-made RLE file formats.
 */

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

// pattern for Life in RLE
typedef struct {
    unsigned int width;
    unsigned int height;
    const char* data;
} rle_pattern_t;

// type of function pointer used to create a preset
typedef unsigned int (*preset_fn_t)(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

/*
 * Function: create_life_blinker_preset
 * --------------------------
 * Game of Life display filled with blinkers
 */
void create_life_blinker_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

/*
 * Function: create_life_small_preset
 * --------------------------
 * Toy Game of Life grid with small oscillators
 */
void create_life_small_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

/*
 * Function: create_life_bunnies
 * --------------------------
 * Game of Life grid with propagating bunnies. From https://conwaylife.com/wiki
 */
void create_life_bunnies(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

/*
 * Function: create_life_karel_177
 * --------------------------
 * Game of Life grid with mandala-style graphics. From https://conwaylife.com/wiki
 */
void create_life_karel_177(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

/*
 * Function: create_life_flying_wing
 * --------------------------
 * Game of Life grid with two flying shapes. From https://conwaylife.com/wiki
 */
void create_life_flying_wing(unsigned int width, unsigned int height, unsigned int padded_width, void* state, color_t *colors);

/*
 * Function: create_random_life_preset
 * --------------------------
 * Create a random preset with the values of each cell determined by using
 * random numbers.
 */
void create_random_life_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

/*
 * Function: create_ww_preset
 * --------------------------
 * WireWorld preset. OR gate and XOR gate feeding into an AND gate.
 */
void create_ww_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

/*
 * Function: create_ww_AND_gates
 * --------------------------
 * WireWorld prest from Golly RLE format.
 */
void create_ww_AND_gates(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);
/*
 * Function: create_ww_nh_multiplication
 * --------------------------
 * WireWorld prest from Golly RLE format.
 */
void create_ww_nh_multiplication(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

/*
 * Function: create_custom_preset
 * --------------------------
 * Placeholder function to indicate use of custom preset.
 */
void create_custom_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors);

#endif
