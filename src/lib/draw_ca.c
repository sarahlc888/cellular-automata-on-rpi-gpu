#include "../../include/draw_ca.h"
#include "gl.h"
// TODO: could separate this by CA type

// TODO: could do bitmap instead of direct map but there's not really a point?



const gate_t or_gate = {
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

const gate_t xor_gate = {
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

const gate_t and_gate = {
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

static void ww_draw_input_lines(unsigned int c, unsigned int r1, unsigned int r2, unsigned int length,
    void *state, unsigned int fb_padded_width, color_t wire_color)
{
    unsigned int (*state_2d)[fb_padded_width] = state;
    for (int j = 0; j < length; j++) {
        state_2d[r1][c + j] = wire_color;
        state_2d[r2][c + j] = wire_color;
    }
}

static void ww_draw_output_line(unsigned int c, unsigned int r, unsigned int length,
    void *state, unsigned int fb_padded_width, color_t wire_color)
{
    unsigned int (*state_2d)[fb_padded_width] = state;
    for (int j = 0; j < length; j++) {
        state_2d[r][c + j] = wire_color;
    }
}

// draw and, or, or xor gate
void ww_draw_gate(gate_t gate_type, int r, int c, unsigned int gate_tail_length,
    unsigned int input1, unsigned int input2,
    void *state, unsigned int fb_padded_width, color_t* colors)
{
    unsigned int (*state_2d)[fb_padded_width] = state;

    // make input
    ww_draw_input_lines(c, r + gate_type.in_row_1, r + gate_type.in_row_2, gate_tail_length, state, fb_padded_width, colors[3]);

    // make gate
    unsigned int base_col = c + gate_tail_length;

    for (int i = 0; i < gate_type.height; i++) {
        for (int j = 0; j < gate_type.width; j++) {
            // if the gate data map is on at (i, j), update state
            if (gate_type.gate_data[i * gate_type.width + j]) {
                state_2d[r + i][base_col + j] = colors[3];
            }
        }
    }
    base_col += gate_type.width;

    // make output tail
    ww_draw_output_line(base_col, r + gate_type.out_row, gate_tail_length, state, fb_padded_width, colors[3]);
    
    // make electrons
    if (input1) {
        state_2d[r + gate_type.in_row_1][c + 1] = colors[1]; // head
        state_2d[r + gate_type.in_row_1][c] = colors[2]; // tail
    }
    if (input2) {
        state_2d[r + gate_type.in_row_2][c + 1] = colors[1]; // head
        state_2d[r + gate_type.in_row_2][c] = colors[2]; // tail
    }
}

void ww_draw_row_wire(int r, int c, int wire_length, 
    void *state, unsigned int fb_padded_width, color_t* colors)
{
    unsigned int (*state_2d)[fb_padded_width] = state;
    for (int j = c; j < wire_length; j++) {
        state_2d[r][j] = colors[3];
    }
    state_2d[r][c + 1] = colors[1]; // head
    state_2d[r][c] = colors[2]; // tail
}

void life_draw_osc(int r, int c, 
    void *state, unsigned int fb_padded_width, color_t* colors)
{
    unsigned int (*state_2d)[fb_padded_width] = state;

    state_2d[r][c - 1] = colors[1];
    state_2d[r][c] = colors[1];
    state_2d[r][c + 1] = colors[1];
}