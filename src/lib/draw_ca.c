/* Sarah Chen
* 03/08/2022
* Code for CS107E Final Project
* 
* This module includes functions to draw various objects within a cellular automata state.
* For Life, this includes oscillators. [TODO: expand.]
* For WireWorld, this includes logic gates. [TODO: expand.]
* 
* TODO: if CA types expand, could separate this by CA type
* TODO: for gates/shapes, could do bitmap instead of direct map, but there is not much point
* TODO: catch case where you attempt to draw outside of fb
* TODO: improve module design to avoid repeated function parameters etc
*/

#include "../../include/draw_ca.h"
#include "../../include/randomHardware.h"
#include "gl.h"
#include "timer.h"
#include "printf.h"

gate_t or_gate = {
    .width = 3, .height = 5,
    .in_row_1 = 1, .in_row_2 = 3, .out_row = 2,
    .gate_data = {
        1, 1, 0,
        0, 0, 1,
        0, 1, 1,
        0, 0, 1,
        1, 1, 0
    }
};

gate_t xor_gate = {
    .width = 6, .height = 7,
    .in_row_1 = 2, .in_row_2 = 4, .out_row = 3,
    .gate_data = {
        0, 1, 1, 0, 0, 0,
        1, 0, 0, 1, 0, 0,
        0, 0, 1, 1, 1, 1,
        0, 0, 1, 0, 1, 1,
        0, 0, 1, 1, 1, 1,
        1, 0, 0, 1, 0, 0,
        0, 1, 1, 0, 0, 0
    }
};

gate_t and_gate = {
    .width = 15, .height = 8,
    .in_row_1 = 0, .in_row_2 = 3, .out_row = 4,
    .gate_data = {
        1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1,
        1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0,
        1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0,
        0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

// Game of life patterns
// still life
pattern_t block = {
    .width = 2, .height = 2,
    .data = {1, 1, 1, 1}
};
// oscillators
pattern_t blinker_1 = {
    .width = 3, .height = 1,
    .data = {1, 1, 1}
};
pattern_t blinker_2 = {
    .width = 1, .height = 3,
    .data = {1, 1, 1}
};
pattern_t toad_1 = { // TODO: toad 2
    .width = 4, .height = 2,
    .data = {
        0, 1, 1, 1,
        1, 1, 1, 0
    }
};
pattern_t beacon_1 = { // TODO: beacon_1
    .width = 4, .height = 4,
    .data = {
        1, 1, 0, 0,
        1, 0, 0, 0,
        0, 0, 0, 1,
        0, 0, 1, 1
    }
};
pattern_t pulsar_1 = { // TODO: pulsar_2, 3
    .width = 13, .height = 13,
    .data = {
        0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1,
        0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
        1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1,
        1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0,
    }
};
// spaceships
pattern_t glider_1 = {
    .width = 3, .height = 3,
    .data = {
        0, 0, 1,
        1, 0, 1,
        0, 1, 1,
    }
};


static gate_t *gate_types[] = {&or_gate, &and_gate, &xor_gate};

static void ww_draw_vert_line(unsigned int r, unsigned int c, unsigned int length,
    void *state, unsigned int fb_padded_width, color_t wire_color)
{
    unsigned int (*state_2d)[fb_padded_width] = state;
    for (int j = 0; j < length; j++) {
        state_2d[r + j][c] = wire_color;
    }
}

/*
 * Function: ww_draw_input_lines
 * --------------------------
 * This function draws the input lines for a gate, starting at the given column `c` and in
 * rows `r1` and `r2`. It proceeds for `length` columns in the positive direction. Drawn
 * lines are in the color `wire_color`.
 * 
 * It alters `state` in place, accessing data using the given `width`.
 * 
 * This does not call `ww_draw_output_line` so that it can execute two instructions within
 * the same loop, which is faster.
 */
static void ww_draw_input_lines(unsigned int c, unsigned int r1, unsigned int r2, unsigned int length,
    void *state, unsigned int fb_padded_width, color_t wire_color)
{
    unsigned int (*state_2d)[fb_padded_width] = state;
    for (int j = 0; j < length; j++) {
        state_2d[r1][c + j] = wire_color;
        state_2d[r2][c + j] = wire_color;
    }
}

/*
 * Function: ww_draw_output_line
 * --------------------------
 * This function draws the output lines for a gate, starting at the given column `c` and in
 * row `r`. It proceeds for `length` columns in the positive direction. Drawn
 * lines are in the color `wire_color`.
 * 
 * It alters `state` in place, accessing data using the given `fb_padded_width`.
 */
static void ww_draw_output_line(unsigned int c, unsigned int r, unsigned int length,
    void *state, unsigned int fb_padded_width, color_t wire_color)
{
    unsigned int (*state_2d)[fb_padded_width] = state;
    for (int j = 0; j < length; j++) {
        state_2d[r][c + j] = wire_color;
    }
}

/*
 * Function: ww_draw_gate
 * --------------------------
 * This function draws the specific gate `gate_ind`, with the top left corner at row `r` 
 * and column `c`. The input and output tails have the specified length, `gate_tail_length`.
 * 
 * Supported gates are AND, OR, and XOR.
 * 
 * The input value for the gate (0 or 1) are in `input1` and `input2`.
 * 
 * It alters `state` in place, accessing data using the given `fb_padded_width`.
 */
static void ww_draw_gate(gate_ind_t gate_ind, int r, int c, 
    unsigned int in_tail_length, unsigned int out_tail_length,
    unsigned int input1, unsigned int input2,
    void *state, unsigned int fb_padded_width, color_t* colors)
{
    gate_t *gate_type = gate_types[gate_ind]; // TODO: why pointers

    unsigned int (*state_2d)[fb_padded_width] = state;
    unsigned int (*gate_2d)[gate_type->width] = (void *) gate_type->gate_data; // TODO: make this work without void *

    // make input
    ww_draw_input_lines(c, r + gate_type->in_row_1, r + gate_type->in_row_2, in_tail_length, state, fb_padded_width, colors[3]);

    // make gate
    unsigned int base_col = c + in_tail_length;

    for (int i = 0; i < gate_type->height; i++) {
        for (int j = 0; j < gate_type->width; j++) {
            // if the gate data map is on at (i, j), update state
            if (gate_2d[i][j] == 1) {
                state_2d[r + i][base_col + j] = colors[3];
            }
        }
    }
    base_col += gate_type->width;

    // make output tail
    ww_draw_output_line(base_col, r + gate_type->out_row, out_tail_length, state, fb_padded_width, colors[3]);
    
    // make electrons
    if (input1) {
        state_2d[r + gate_type->in_row_1][c + 1] = colors[1]; // head
        state_2d[r + gate_type->in_row_1][c] = colors[2]; // tail
    }
    if (input2) {
        state_2d[r + gate_type->in_row_2][c + 1] = colors[1]; // head
        state_2d[r + gate_type->in_row_2][c] = colors[2]; // tail
    }
}

// TODO: consolidate w the output lines one
static void ww_draw_row_wire(int r, int c, int wire_length, 
    void *state, unsigned int fb_padded_width, color_t* colors)
{
    unsigned int (*state_2d)[fb_padded_width] = state;
    for (int j = c; j < c + wire_length; j++) {
        state_2d[r][j] = colors[3];
    }
    state_2d[r][c + 1] = colors[1]; // head
    state_2d[r][c] = colors[2]; // tail
}

// draw specified pattern in GOL
static void life_draw_pattern(pattern_t *pattern, int r, int c, 
    void *state, unsigned int fb_padded_width, color_t* colors)
{
    // TODO: why does this only work when using a pointer to pattern

    unsigned int (*state_2d)[fb_padded_width] = state;
    // TODO: remove void * ? error: initialization of 'unsigned int (*)[(sizetype)(pattern->width)]' from incompatible pointer type 'unsigned int *'
    unsigned int (*pattern_2d)[pattern->width] = (void *) pattern->data;
    for (int i = 0; i < pattern->height; i++) {
        for (int j = 0; j < pattern->width; j++) {
            if (pattern_2d[i][j] == 1) {
                state_2d[r + i][c + j] = colors[1];
            }
        }
    }
}

/*
 * Function: life_draw_osc
 * --------------------------
 * This function draws a Life oscillator at the given row `r` and column `c`.
 * 
 * It alters `state` in place, accessing data using the given `fb_padded_width`.
 */
static void life_draw_osc(int r, int c, 
    void *state, unsigned int fb_padded_width, color_t* colors)
{

    unsigned int (*state_2d)[fb_padded_width] = state;

    state_2d[r][c - 1] = colors[1];
    state_2d[r][c] = colors[1];
    state_2d[r][c + 1] = colors[1];
}

/*
 * Function: create_random_life_preset
 * --------------------------
 * This function populates `state` with a random preset state for Life.
 */
void create_random_life_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors)
{
    random_init();
    timer_delay(3); // allow for adequate time before querying for random numbers
    unsigned int (*state_2d)[padded_width] = state;

    for (int i = 5; i < width - 5; i++) {
        for (int j = 5; j < height - 5; j++) {
            // get a number between 0 and 5
            unsigned int num = random_getNumber(0, 100);

            // if number is greater than 4, make the cell alive
            num %= 5; 
            if (num >= 3) {
                state_2d[i][j] = colors[1];
            }
        }
    }
}

/*
 * Function: create_life_preset
 * --------------------------
 * This function populates `state` with a random preset state for Life.
 */
void create_life_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors)
{
    for (int i = 5; i < width - 5; i += 5) {
        for (int j = 5; j < height - 5; j += 5) {
            life_draw_osc(i, j, state, padded_width, colors);
        }
    }
}

/*
 * Function: create_life_preset2
 * --------------------------
 * This function populates `state` with a custom preset state for Life that uses more patterns.
 * 
 * This requires at least a 32x32 screen
 */
void create_life_preset2(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors)
{
    int cur_row = 5;
    int cur_col = 5;
    life_draw_pattern(&block, cur_row, cur_col, state, padded_width, colors);        
    cur_row += 6;
    life_draw_pattern(&blinker_1, cur_row, cur_col, state, padded_width, colors);        
    cur_row += 6;
    life_draw_pattern(&blinker_2, cur_row, cur_col, state, padded_width, colors);        
    cur_row += 6;
    life_draw_pattern(&toad_1, cur_row, cur_col, state, padded_width, colors);        
    cur_row += 6;

    cur_row = 5;
    cur_col = 16;
    life_draw_pattern(&beacon_1, cur_row, cur_col, state, padded_width, colors);        
    cur_row += 16;
    life_draw_pattern(&pulsar_1, cur_row, cur_col, state, padded_width, colors);  

    cur_row = 5;
    cur_col = 30;
    life_draw_pattern(&glider_1, cur_row, cur_col, state, padded_width, colors);        
}

/*
 * Function: create_ww_preset
 * --------------------------
 * This function populates `state` with a custom preset state for WireWorld.
 */
void create_ww_preset(unsigned int width, unsigned int height, unsigned int padded_width, void *state, color_t *colors)
{

    ww_draw_gate(WW_OR, 2, 2, 
        3, 4, 1, 1,
        state, padded_width, colors);
    ww_draw_gate(WW_XOR, 8, 2, 
        2, 2, 1, 0,
        state, padded_width, colors);


    ww_draw_vert_line(4, 12, 3, state, padded_width, colors[3]);
    ww_draw_vert_line(9, 12, 3, state, padded_width, colors[3]);
    ww_draw_gate(WW_AND, 6, 12, 
        2, 1, 0, 0,
        state, padded_width, colors);
}