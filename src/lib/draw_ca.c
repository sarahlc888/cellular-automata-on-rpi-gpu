/* Sarah Chen and Avi Udash
 * 03/08/2022
 * Code for CS107E Final Project
 *
 * This module includes structs for various objects/patterns for the cellular
 * automata presetes. It also includes the functions needed to draw passed objects
 * as well as higher-level functions to draw entire presets (or read them from
 * RLE file format).
 * 
 * For Life, patterns include oscillators, gliders, and more. For WireWorld, 
 * patterns are primarily logic gates.
 */

#include "../../include/draw_ca.h"
#include "../../include/randomHardware.h"
#include "gl.h"
#include "printf.h"
#include "strings.h"
#include "timer.h"

// Wireworld gate types
gate_t or_gate = {.width = 3,
                  .height = 5,
                  .in_row_1 = 1,
                  .in_row_2 = 3,
                  .out_row = 2,
                  .gate_data = {
                      1, 1, 0, 
                      0, 0, 1, 
                      0, 1, 1, 
                      0, 0, 1, 
                      1, 1, 0
                  }};

gate_t xor_gate = {.width = 6,
                   .height = 7,
                   .in_row_1 = 2,
                   .in_row_2 = 4,
                   .out_row = 3,
                   .gate_data = {
                        0, 1, 1, 0, 0, 0,
                        1, 0, 0, 1, 0, 0,
                        0, 0, 1, 1, 1, 1, 
                        0, 0, 1, 0, 1, 1, 
                        0, 0, 1, 1, 1, 1, 
                        1, 0, 0, 1, 0, 0, 
                        0, 1, 1, 0, 0, 0
                   }};

gate_t and_gate = {.width = 15,
                   .height = 8,
                   .in_row_1 = 0,
                   .in_row_2 = 3,
                   .out_row = 4,
                   .gate_data = {
                        1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 
                        0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0,
                        0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
                        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 
                        1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0,
                        1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 
                        0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                   }};

static gate_t *gate_types[] = {&or_gate, &and_gate, &xor_gate};


// Game of life patterns
// still life
pattern_t block = {.width = 2, .height = 2, .data = {1, 1, 1, 1}};
// oscillators
pattern_t blinker_1 = {.width = 3, .height = 1, .data = {1, 1, 1}};
pattern_t blinker_2 = {.width = 1, .height = 3, .data = {1, 1, 1}};
pattern_t toad_1 = { 
    .width = 4,
    .height = 2,
    .data = {
        0, 1, 1, 1, 
        1, 1, 1, 0
    }};
pattern_t beacon_1 = { 
    .width = 4,
    .height = 4,
    .data = {
        1, 1, 0, 0, 
        1, 0, 0, 0, 
        0, 0, 0, 1, 
        0, 0, 1, 1
    }};
pattern_t pulsar_1 = { 
    .width = 13,
    .height = 13,
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
    }};
// spaceships
pattern_t glider_1 = {.width = 3,
                      .height = 3,
                      .data = {
                          0, 0, 1, 
                          1, 0, 1,
                          0, 1, 1,
                      }};


// RLE patterns for Game of Life and WireWorld, taken directly from outside sources (see links)
// https://conwaylife.com/wiki/Bunnies
rle_pattern_t bunnies = {
    .width = 6, .height = 6, .data = "bo4b$2obo2b$4b2o$o2bo2b$o5b$o!"};

// https://conwaylife.com/wiki/Karel%27s_p177
rle_pattern_t karel_177_osc = {
    .width = 46,
    .height = 46,
    .data = "16bo12bo$9b2o24b2o$8b3o3b2o14b2o3b3o$14b2ob2o8b2ob2o$16bo12bo4$"
            "2bo40bo$b2o40b2o$b2o40b2o4$2b2o38b2o$2b2o38b2o$o3bo36bo3bo$"
            "3bo38bo$3bo38bo9$3bo38bo$3bo38bo$o3bo36bo3bo$2b2o38b2o$2b2o38b2o4$"
            "b2o40b2o$b2o40b2o$2bo40bo4$16bo12bo$14b2ob2o8b2ob2o$"
            "8b3o3b2o14b2o3b3o$9b2o24b2o$16bo12bo!"};

// https://conwaylife.com/wiki/Flying_wing
rle_pattern_t flying_wing = {
    .width = 159,
    .height = 90,
    .data =
        "13b3o11b3o99b3o11b3o$12bo3bo9bo3bo97bo3bo9bo3bo$"
        "11b2o4bo7bo4b2o95b2o4b"
        "o7bo4b2o$10bobob2ob2o5b2ob2obobo93bobob2ob2o5b2ob2obobo$"
        "9b2obo4bob2ob 2obo4bob2o35b3o15b3o35b2obo4bob2ob2obo4bob2o "
        "$8bo4bo3bo2bobo2bo3bo4bo4b"
        "o28bo3bo13bo3bo28bo4bo4bo3bo2bobo2bo3bo4bo$"
        "20bobo15b3ob2o23b2o4bo11bo 4b2o23b2ob3o15bobo "
        "$8b2o7b2obobob2o7b2o2b2o3bob2o20bobob2ob2o3b3o3b2ob "
        "2obobo20b2obo3b2o2b2o7b2obobob2o7b2o "
        "$20bobo13b2o2bo4b5o15b2obo4bob2ob 3ob2obo4bob2o15b5o4bo2b2o13bobo "
        "$14b3o9b3o5bo2bo5bobobob2ob2o5bo4bo4bo "
        "3bo4bobo4bo3bo4bo4bo5b2ob2obobobo5bo2bo5b3o9b3o "
        "$14bo3bo9bo6bo8bo3bo3bo "
        "b2o2b3o15bo5bo15b3o2b2obo3bo3bo8bo6bo9bo3bo$"
        "14bobo4b3o26bo4b3ob3o2b2o 7b2o9b2o7b2o2b3ob3o4bo26b3o4bobo "
        "$20bo2bo4b2o19bo3bobo5b2o33b2o5bobo3bo 19b2o4bo2bo "
        "$23bo30bo6bo2bo29bo2bo6bo30bo$19bo3bo39bo31bo39bo3bo$19bo3b "
        "o111bo3bo$14bo8bo111bo8bo$14bobo3bobo113bobo3bobo$14b2o127b2o4$"
        "12bo23b 3o11b3o53b3o11b3o23bo "
        "$12bobo20bo3bo9bo3bo51bo3bo9bo3bo20bobo$12b2o20b "
        "2o4bo7bo4b2o49b2o4bo7bo4b2o20b2o "
        "$33bobob2ob2o5b2ob2obobo47bobob2ob2o5b 2ob2obobo "
        "$32b2obo4bob2ob2obo4bob2o45b2obo4bob2ob2obo4bob2o$31bo4bo3bo "
        "2bobo2bo3bo4bo43bo4bo3bo2bobo2bo3bo4bo "
        "$10bo32bobo67bobo32bo$10bobo18b "
        "2o7b2obobob2o7b2o43b2o7b2obobob2o7b2o18bobo "
        "$10b2o31bobo67bobo31b2o$37b 3o9b3o55b3o9b3o "
        "$37bo3bo9bo55bo9bo3bo$37bobo4b3o65b3o4bobo$8bo34bo2bo4b "
        "2o53b2o4bo2bo34bo"
        "$8bobo35bo65bo35bobo$8b2o32bo3bo65bo3bo32b2o$42bo3bo 65bo3bo "
        "$37bo8bo65bo8bo$37bobo3bobo67bobo3bobo$6bo30b2o81b2o30bo$6bobo "
        "141bobo"
        "$6b2o143b2o2$35bo87bo$35bobo83bobo$4bo30b2o85b2o30bo$4bobo145bo "
        "bo$4b2o147b2o2$33bo91bo$33bobo87bobo$2bo30b2o89b2o30bo$2bobo149bobo$"
        "2b2o151b2o2"
        "$31bo95bo$31bobo91bobo$o30b2o93b2o30bo$obo153bobo$2o155b2o2$"
        "29bo99bo"
        "$29bobo95bobo$29b2o97b2o$3o153b3o$obo153bobo$3o153b3o$27bo103b"
        "o$6b2o19bobo99bobo19b2o$8bo18b2o101b2o18bo$6b3o141b3o$5bob2o141b2obo$"
        "4bobo145bobo"
        "$4bobo18bo107bo18bobo$25bobo103bobo$9b2o14b2o105b2o14b2o$ "
        "8bo2bo135bo2bo"
        "$9b2o137b2o2$23bo111bo$12b2o9bobo107bobo9b2o$11bo2bo8b2o "
        "109b2o8bo2bo"
        "$12b2o131b2o3$21bo115bo$21bobo111bobo$17b2o2b2o113b2o2b2o$ "
        "17b2o121b2o!"};

// http://golly.sourceforge.net/
rle_pattern_t ww_AND_gate_pattern = {
    .width = 60,
    .height = 60,
    .data =
        "14.A5C$20.C$10.A5C5.3C$16.C.C.C3.C$17.3C4.C$18.C.C.C.C$21.3C$22.C.10C"
        "11$4.A5C19.6C$10.C24.C$6C5.3C11.A5C5.3C$6.C.C.C3.C16.C.C.C3.C$7.3C4.C"
        "17.3C4.C$8.C.C.C.C18.C.C.C.C$11.3C22.3C$12.C.10C13.C.10C6$4.CA4C19.A5C"
        "$10.C24.C$A5C5.3C11.CA4C5.3C$6.C.C.C3.C16.C.C.C3.C$7.3C4.C17.3C4.C$8.C"
        ".C.C.C18.C.C.C.C$11.3C22.3C$12.C.10C13.C.10C"};

// http://golly.sourceforge.net/
rle_pattern_t ww_nh_multiplication = {
    .width = 400,
    .height = 200,
    .data =
        ".C3.C..C3.C.CC3.C3.C..3C$C.C.C.C.CC.CC.C.C.C.C.C.C..C$C3.C.C.C.C.C."
        "CC..C.C.C4.C$C.C.C.C.C3.C.C3.3C.C.C..C$.C3.C..C3.C.C3.C.C..C3.C4$4."
        "23C.C.C3.C.4C.10C.42C$3.C23.3C.A4C4.C10.C42.C$4.21C3.C.B3.C.C..3C.."
        "C3.3C..41C$25.C.C5.C3.C..C..A.C.C4.C$4.21C..C4.C..CC..4CAB3.3C.."
        "32CBACCBACCB$3.C23.C3.C..C3.C.C4.A5.C42.A$4.17C3.C..C..C3.C3.C3.CC.."
        "CC.A..ABCCABCCABCCABCCABCCABCCABCCABCCABCCABCC..C$21.C.C.C.3C4.C4.C."
        "C..C3.CAB41.A.C$4.17C..C.C..C3.C..C4.3C.C.C..A.CAB."
        "CABCCABCCABCCABCCABCCABCCABCCABCCAB..B.B$3.C19.C.C..7C4.C.A3.4C5.C35."
        "C.C.A$4.17C..C.C.C4.C3.A3.B5.C6.B.."
        "ACCBACCBACCBACCBACCBACCBACCBACCBAC..C.C$21.C.C.C.C.C.CC.CB.C.3C..CC7."
        "A.B36.A.C$4.17C..C..5C..CC.C3.C..B.C.BC..B.C.."
        "CCABCCABCCABCCABCCABCC4.CBACCBACCB..B$3.C19.C3.A.C.C..3C4.CA3.A.."
        "ABBC25.A..C12.A$4.16C3.C3.B4.C..C13.B.B.BACCBACCBACCBACCBACCBACCB4."
        "ABCCABCCAB..C$20.C..C..C.A4.C.C5.ACCBA3C43.C.C$4.16C..CC3.C..3C..C4."
        "B9.BACCBA3CBACCBACCBACCBACCBAC3.C3.BACCBAC..B$3.C17.C.CC4.C5.C3.C4."
        "CCBACC28.C.C.A.C9.A$4.15C..C3.C..3C.C.3C.C..CAA7."
        "CABCCABCCABCCABCCABCCABCC..4B3.CABCCAB..C$19.C.C..3C..C.3C.C..A.C.."
        "B6.C25.A..C..A9.C.C$4.15C..C.C.C.CC.C.C..B.CACC.C.C6."
        "BACCBACCBACCBACCBACCBACC..B3C.C..BACCBAC..B$3.C17.C.C10.A.C.A.CC3."
        "C29.B3.C..C.C9.A$4.17C3.10C.C6.ABC.ABCCABCCABCCABCCABCCABCCABCCA5."
        "AB3.CABCCABCC10$85.23C$84.C23.C$84.C..21C$84.C.C$84.C..31C$3C.C3.C."
        "CC3.C..C3.C.CC..3C.CC50.C33.C$C4.C.C..C.C.C.C.CC..C.C.C.C3.C.C49.C.."
        "31C$CC4.C3.CC..C.C.C.C.C.C.C.CC..C.C49.C.C$C4.C.C..C3.3C.C..CC.C.C."
        "C3.C.C49.C..BACCBACCBACCBACCBACCBACCBACCBAC$3C.C3.C.C3.C.C.C3.C.CC.."
        "3C.CC50.C33.C$84.C..CABCCABCCABCCABCCABCCABCCABCCAB$84.C.C$84.C.."
        "BACCBACCBACCBACCBACCBACCBACCBAC$84.C33.C$85."
        "3CABCCABCCABCCABCCABCCABCCABCCAB$87.C$87.C5.C3.C5.3C..C$87.C4.C.C.C."
        "C6.C..C$55.3C..C10.CC3.C..3C.3C.C4.C.C.C.C.CC..CC..C$55.C3.C.C8.C4.C."
        "C..C..C3.C4.3C.C.C6.C..C$18.C3.C5.C3.3C20.CC..C.C8.C.CC.C.C..C..CC.."
        "C4.C.C..C5.3C.3C$17.C.C.C.C4.C5.C20.C3.3C8.C.C..3C..C..C3.C$17.C3.C."
        "C.CC.3C..CC20.C3.C.C9.CC..C.C..C..3C.C$17.C.C.C.C4.C.C3.C52.C$18.C3."
        "C5.3C.3C21.C.C3.C.12C.10C$52.6C.A4C12.C$4.31C16.C4.C.B3.C.C10.3C.."
        "C11.CC..3C..C6.C$3.C47.C3.C5.C3.C10.C..A.C10.C.C3.C..C5.C.C$4.31C16."
        "C3.C4.C..CC10.4CAB11.CC3.CC..C..CC.C.C$35.16C4.C3.C..C11.C.C4.A10.C."
        "C3.C..C5.C.C$4.31C17.C..C..C3.C11.C3.CC..CC.A6.CC..3C.3C5.C$3.C47.C."
        "C.3C4.C12.C.C..C3.CAB$4.31C16.C.C..C3.C..C12.3C.C.C..A.CAB."
        "CABCCABCCABCCABCCABCCABCCABCC$35.C15.C.C..7C12.C.A3.4C5.C29.A$4."
        "31C16.C.C.C4.C3.A11.B5.C6.B3.CCBACCBACCBACCBACCBACCBACCB$3.C47.C.C.C."
        "C.CC.CB.C9.3C..CC7.A..A$4.31C16.C..5C..CC.C11.C..B.C.BC..B.C3."
        "BCCABCCABCCABCCABCCABCCABCC$35.C15.C3.A.C.C..3C12.CA3.A..ABBC31.A$4."
        "31C16.C3.B4.C..C21.B.B.B3.CCBACCBACCBACCBACCBACCBACCB$3.C47.C..C.A4."
        "C.C13.ACCBA3C4.A..A$4.48C3.C..3C..C12.B12.C.B$51.CC4.C5.C11.C14.C$53."
        "C..3C.C.3C9.C$52.3C..C.3C.C10.A$51.C.C.CC.C.C..B.BACCBA3CAC$51.C10.A."
        "C9.A.C$52.10C.C12.C$76.C16.C..C4.C3.C..C..C$76.C15.C.C.C3.C.C.C.C.C."
        "C$.C..C3.C4.3C.C3.C.C.C.3C.CC..3C.3C.CC31.C15.C3.C3.C.C.C3.CC$C.C.CC."
        ".C5.C..CC..C.C.C.C3.C.C..C..C3.C.C30.C15.C.C.C3.C.C.C.C.C.C$C.C.C.C."
        "C5.C..C.C.C.C.C.CC..CC3.C..CC..CC31.C16.C..3C..C3.C..C..C$3C.C..CC5."
        "C..C..CC.C.C.C3.C.C..C..C3.C.C30.C$C.C.C3.C4.3C.C3.C..C..3C.C.C..C.."
        "3C.C.C30.C11.CBACCBACCBACCBACCBA3CBACCBA3.A$76.C5.ACCBAC28.C.C.B$10."
        "C65.C..ABB7.ABCCABCCABCCABCCABCCABCCA..4C$9.A.C64.C.C..C6.C25.B..C.."
        "B$9.B.B8.A56.CC.C.C6.CBACCBACCBACCBACCBACCBAC..4C.A$9.C.A6.CB.C56."
        "CA3.A29.C3.A..C$9.C..7C.C59.BCC.BCCABCCABCCABCCABCCABCCABCCAB5.BC$9."
        "C.C6.3C$9.C.C7.C$9.C.C8.12C$10.C"};


// Functions to draw further patterns for various automata
/*
 * Function: ww_draw_vert_line
 * --------------------------
 * This function draws a vertical line starting at the given (`r`, `c`) with
 * the given `length`. Drawn lines are in the color `wire_color`.
 *
 * It alters `state` in place, accessing data using the given `fb_padded_width`.
 */
static void ww_draw_vert_line(unsigned int r, unsigned int c,
                              unsigned int length, void *state,
                              unsigned int fb_padded_width,
                              color_t wire_color) {
  unsigned int(*state_2d)[fb_padded_width] = state;
  for (int j = 0; j < length; j++) {
    state_2d[r + j][c] = wire_color;
  }
}

/*
 * Function: ww_draw_input_lines
 * --------------------------
 * This function draws the input lines for a gate, starting at the given column
 * `c` and in rows `r1` and `r2`. It proceeds for `length` columns in the
 * positive direction. Drawn lines are in the color `wire_color`.
 *
 * It alters `state` in place, accessing data using the given `fb_padded_width`.
 *
 * This does not call `ww_draw_output_line` so that it can execute two
 * instructions within the same loop, which is faster.
 */
static void ww_draw_input_lines(unsigned int c, unsigned int r1,
                                unsigned int r2, unsigned int length,
                                void *state, unsigned int fb_padded_width,
                                color_t wire_color) {
  unsigned int(*state_2d)[fb_padded_width] = state;
  for (int j = 0; j < length; j++) {
    state_2d[r1][c + j] = wire_color;
    state_2d[r2][c + j] = wire_color;
  }
}

/*
 * Function: ww_draw_output_line
 * --------------------------
 * This function draws the output lines for a gate, starting at the given column
 * `c` and in row `r`. It proceeds for `length` columns in the positive
 * direction. Drawn lines are in the color `wire_color`.
 *
 * It alters `state` in place, accessing data using the given `fb_padded_width`.
 */
static void ww_draw_output_line(unsigned int c, unsigned int r,
                                unsigned int length, void *state,
                                unsigned int fb_padded_width,
                                color_t wire_color) {
  unsigned int(*state_2d)[fb_padded_width] = state;
  for (int j = 0; j < length; j++) {
    state_2d[r][c + j] = wire_color;
  }
}

/*
 * Function: ww_draw_gate
 * --------------------------
 * This function draws the specific gate `gate_ind`, with the top left corner at
 * row `r` and column `c`. The input and output tails have the specified length,
 * `in_tail_length` and `out_tail_length`.
 *
 * Supported gates are AND, OR, and XOR.
 *
 * The input value for the gate (0 or 1) are in `input1` and `input2`.
 *
 * It alters `state` in place, accessing data using the given `fb_padded_width`.
 */
static void ww_draw_gate(gate_ind_t gate_ind, int r, int c,
                         unsigned int in_tail_length,
                         unsigned int out_tail_length, unsigned int input1,
                         unsigned int input2, void *state,
                         unsigned int fb_padded_width, color_t *colors) {
  gate_t *gate_type = gate_types[gate_ind];

  unsigned int(*state_2d)[fb_padded_width] = state;
  unsigned int(*gate_2d)[gate_type->width] =
      (void *)gate_type->gate_data;

  // make input
  ww_draw_input_lines(c, r + gate_type->in_row_1, r + gate_type->in_row_2,
                      in_tail_length, state, fb_padded_width, colors[3]);

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
  ww_draw_output_line(base_col, r + gate_type->out_row, out_tail_length, state,
                      fb_padded_width, colors[3]);

  // make electrons
  if (input1) {
    state_2d[r + gate_type->in_row_1][c + 1] = colors[1]; // head
    state_2d[r + gate_type->in_row_1][c] = colors[2];     // tail
  }
  if (input2) {
    state_2d[r + gate_type->in_row_2][c + 1] = colors[1]; // head
    state_2d[r + gate_type->in_row_2][c] = colors[2];     // tail
  }
}

/*
 * Function: ww_draw_row_wire
 * --------------------------
 * This function draws a horizontal line starting at the given (`r`, `c`) with
 * the given `length`. Drawn lines are in the color `wire_color`.
 *
 * It alters `state` in place, accessing data using the given `fb_padded_width`.
 * 
 * It draws an electron at the left end of the wire.
 */
static void ww_draw_row_wire(int r, int c, int wire_length, void *state,
                             unsigned int fb_padded_width, color_t *colors) {
  unsigned int(*state_2d)[fb_padded_width] = state;
  for (int j = c; j < c + wire_length; j++) {
    state_2d[r][j] = colors[3];
  }
  state_2d[r][c + 1] = colors[1]; // head
  state_2d[r][c] = colors[2];     // tail
}

/*
 * Function: life_draw_pattern
 * --------------------------
 * This function draws the specified `pattern` for the game of life at 
 * the given (`r`, `c`) using the given `colors`.
 * 
 * It alters `state` in place, accessing data using the given `fb_padded_width`.
 */
static void life_draw_pattern(pattern_t *pattern, int r, int c, void *state,
                              unsigned int fb_padded_width, color_t *colors) {
  unsigned int(*state_2d)[fb_padded_width] = state;
  unsigned int(*pattern_2d)[pattern->width] = (void *)pattern->data;
  for (int i = 0; i < pattern->height; i++) {
    for (int j = 0; j < pattern->width; j++) {
      if (pattern_2d[i][j] == 1) {
        state_2d[r + i][c + j] = colors[1];
      }
    }
  }
}

/* 
 * Function: draw_rle_pattern
 * --------------------------
 * Draw specified cellular automata pattern in RLE format.
 *
 * RLE Format is a long string with following format:
 * <run_count><tag>
 * where <run_count> indicates how many of the <tag> type is printed.
 * <tag> is the type of cell, 'b' for dead, 'o' for alive, '$' for end of line.
 *
 * For CA with more than 2 states, '.' is bg color, 'A' is color 1, 'B' is color
 * 2, etc...
 *
 * The last <run_count><tag> is followed by an optional'!' to indicate finished
 *
 * Note: - if run_count is missing/is 0, then there is only one of the <tag>
 *          type
 *
 * Read more about the format here:
 * http://golly.sourceforge.net/Help/formats.html#rle
 */
static void draw_rle_pattern(rle_pattern_t *pattern, int r, int c, void *state,
                             unsigned fb_padded_width, color_t *colors) {
  unsigned int(*state_2d)[fb_padded_width] = state;
  unsigned x = 0;
  unsigned y = 0;

  // use 'rest' instead of 'pattern->data' since we don't want to manipulate
  // pattern
  const char *rest = pattern->data;
  char type = *rest;

  while (type != '\0' && type != '!') {
    // find run length
    unsigned run_length = strtonum(rest, &rest);
    type = *rest; // get type of cell

    // increment string
    rest++;

    // handle case where run_length wasn't included
    if (run_length == 0) {
      run_length = 1;
    }

    // loop the length of the run, printing correct cell type
    for (int i = 0; i < run_length; i++) {
      if (type == 'b' || type == '.') { // dead cell
        state_2d[r + y][c + x] = colors[0];
        x++;
      } else if (type == 'o') { // alive cell
        state_2d[r + y][c + x] = colors[1];
        x++;
      } else if (type >= 'A' && type <= 'C') { // rules with more than 2 states
        state_2d[r + y][c + x] = colors[(type - 'A') + 1];
        x++;
      } else if (type == '$') { // newline
        y++;
        x = 0;
      }
    }
  }
}

// Functions to make presets for life and wireworld
/*
 * Function: create_random_life_preset
 * --------------------------
 * This function populates `state` with a random preset state for Life.
 */
void create_random_life_preset(unsigned int width, unsigned int height,
                               unsigned int padded_width, void *state,
                               color_t *colors) {
  random_init();
  timer_delay(3); // allow for adequate time before querying for random numbers
  unsigned int(*state_2d)[padded_width] = state;

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
void create_life_preset(unsigned int width, unsigned int height,
                        unsigned int padded_width, void *state,
                        color_t *colors) {
  for (int i = 5; i < width - 5; i += 5) {
    for (int j = 5; j < height - 5; j += 5) {
      life_draw_pattern(&blinker_1, i, j, state, padded_width, colors);
    }
  }
}

/*
 * Function: create_life_preset2
 * --------------------------
 * This function populates `state` with a custom preset state for Life that uses
 * more patterns.
 *
 * This pattern is very small but requires at least a 32x32 screen.
 */
void create_life_preset2(unsigned int width, unsigned int height,
                         unsigned int padded_width, void *state,
                         color_t *colors) {
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
 * Function: create_life_bunnies
 * --------------------------
 * This function populates `state` with propagating bunnies for the Game
 * of Life.
 */
void create_life_bunnies(unsigned int width, unsigned int height,
                         unsigned int padded_width, void *state,
                         color_t *colors) {
  draw_rle_pattern(&bunnies, 60, 60, state, padded_width, colors);
}

/*
 * Function: create_life_karel_177
 * --------------------------
 * This function populates `state` with mandala-style graphics for the Game
 * of Life.
 */
void create_life_karel_177(unsigned int width, unsigned int height,
                           unsigned int padded_width, void *state,
                           color_t *colors) {
  for (int y = 10; y < width;
       y += karel_177_osc.height + karel_177_osc.height / 2) {
    for (int x = 10; x < width;
         x += karel_177_osc.width + karel_177_osc.width / 2) {
      draw_rle_pattern(&karel_177_osc, y, x, state, padded_width, colors);
    }
  }
}

/*
 * Function: create_life_flying_wing
 * --------------------------
 * This function populates `state` with two wings for the Game of Life.
 */
void create_life_flying_wing(unsigned int width, unsigned int height,
                             unsigned int padded_width, void *state,
                             color_t *colors) {
  unsigned x = (width / 4) - (flying_wing.width / 2);
  unsigned y = height - flying_wing.height - 20;
  draw_rle_pattern(&flying_wing, y, x, state, padded_width, colors);

  x = (width * 3 / 4) - (flying_wing.width / 2);
  draw_rle_pattern(&flying_wing, y, x, state, padded_width, colors);
}

/*
 * Function: create_ww_preset
 * --------------------------
 * This function populates `state` with a custom preset state for WireWorld.
 */
void create_ww_preset(unsigned int width, unsigned int height,
                      unsigned int padded_width, void *state, color_t *colors) {

  ww_draw_gate(WW_OR, 2, 2, 3, 4, 1, 1, state, padded_width, colors);
  ww_draw_gate(WW_XOR, 8, 2, 2, 2, 1, 0, state, padded_width, colors);

  ww_draw_vert_line(4, 12, 3, state, padded_width, colors[3]);
  ww_draw_vert_line(9, 12, 3, state, padded_width, colors[3]);
  ww_draw_gate(WW_AND, 6, 12, 2, 1, 0, 0, state, padded_width, colors);
}

/*
 * Function: create_ww_AND_gates
 * --------------------------
 * This function draws AND gates with various input states.
 */
void create_ww_AND_gates(unsigned int width, unsigned int height,
                         unsigned int padded_width, void *state,
                         color_t *colors) {
  draw_rle_pattern(&ww_AND_gate_pattern, 2, 2, state, padded_width, colors);
}

/*
 * Function: create_ww_nh_multiplication
 * --------------------------
 * Pattern taken from https://github.com/jimblandy/golly/blob/master/src/Patterns/WireWorld/NylesHeise.mcl
 */
void create_ww_nh_multiplication(unsigned int width, unsigned int height,
                                 unsigned int padded_width, void *state,
                                 color_t *colors) {
  draw_rle_pattern(&ww_nh_multiplication, 2, 2, state, padded_width, colors);
}

/*
 * Function: create_custom_preset
 * --------------------------
 * This function is intended as a placeholder to run on etch-a-sketch input, so
 * it should not draw anything on the screen
 */
void create_custom_preset(unsigned int width, unsigned int height,
                          unsigned int padded_width, void *state,
                          color_t *colors) {}
