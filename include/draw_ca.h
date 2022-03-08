#ifndef DRAWCA_H
#define DRAWCA_H

#include "gl.h"


typedef struct {
    unsigned int width;
    unsigned int height;
    unsigned int in_row_1;
    unsigned int in_row_2;
    unsigned int out_row;
    unsigned int gate_data[];
} gate_t;

void ww_draw_gate(gate_t gate_type, int r, int c, unsigned int gate_tail_length,
    unsigned int input1, unsigned int input2,
    void *state, unsigned int fb_padded_width, color_t* colors);

void ww_draw_row_wire(int r, int c, int wire_length, 
    void *state, unsigned int fb_padded_width, color_t* colors);

void life_draw_osc(int r, int c, 
    void *state, unsigned int fb_padded_width, color_t* colors);

#endif