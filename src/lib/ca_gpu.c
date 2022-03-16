/* Sarah Chen
 * 03/08/2022
 * Code for CS107E Final Project
 *
 * This module supports representation and simulation of cellular automata.
 *
 * Currently supported automata:
 * - The Game of Life
 * - WireWorld
 *
 * Currently, the state is directly stored within the frame buffers. The frame
 * buffer is double-buffered, so the code considers one buffer at a time to edit
 * the other. States are therefore implicitly represented as colors, and an
 * arbitrary number of states are supported because they depend on a
 * client-specified color array.
 *
 * Another approach would require having 2 further arrays to represent states,
 * which are then copied into the framebuffer. So far, this has not seemed to be
 * necessary.
 *
 * Possible additions to the module, if time allows:
 * - Correct behavior at borders -- need to make this bigger than the space ? to
 * simulate infinity? (currently, behavior is WRONG when hitting the borders
 * because there is no bounds catching)
 * - Add further CA
 *      - Support rule tables (http://www.mirekw.com/ca/rullex_rtab.html)
 *        (https://github.com/GollyGang/ruletablerepository/blob/gh-pages/src/read_ruletable.cpp)
 *      - Add Langston's Loop or another loop CA
 *      - Possibly support more granular neighbor information (which neighbors
 * are on), if necessary
 */

#include "../../include/ca_gpu.h"
#include "../../include/button.h"
#include "../../include/draw_ca.h"
#include "../../include/mailbox_functions.h"
#include "../../include/qpu.h"
#include "../../include/read_write_ca.h"
#include "assert.h"
#include "fb.h"
#include "gl.h"
#include "malloc.h"
#include "printf.h"
#include "timer.h"

#define SIZE(x) sizeof(x) / sizeof(x[0])

// struct to store information about the current cellular automata simulation
typedef struct {
  unsigned int mode; // which cellular automata to run

  unsigned int height;          // height of the physical screen
  unsigned int width;           // width of the physical screen
  unsigned int bordered_height; // height of the physical screen
  unsigned int bordered_width;  // width of the physical screen
  unsigned int padded_width;    // width of the physical screen plus padding
  color_t *state_colors;        // colors for various states

  unsigned int update_ms; // ms between state updates

  // pointers to the framebuffer
  unsigned int *cur_state;
  unsigned int *next_state;

} ca_config_t;

static volatile ca_config_t
    ca; // TODO: is volatile necessary? will something else change the code?

// CA mode table links titles, styled titles, and update functions
// TODO: why do I need these titles?
static unsigned int game_of_life_update_pix(unsigned int r, unsigned int c,
                                            void *state);
static unsigned int wireworld_update_pix(unsigned int r, unsigned int c,
                                         void *state);

static void gpu_gol_update_state(void *prev, void *next);
static void cpu_ww_update_state(void *prev, void *next);

static const ca_option_t ca_modes[] = {
    {"life", "Conway", gpu_gol_update_state},
    {"wireworld", "WireWorld!", cpu_ww_update_state},
};

/*
 * Function: ca_init
 * --------------------------
 * Initialize a cellular automaton simulation with mode `ca_mode_t`.
 * Specify the screen dimensions `screen_width` and `screen_height`.
 * Specify the `colors` used in the automaton.
 * The first color passed in `colors` is presumed to be the background color.
 * Specify the `update_delay` in milliseconds.
 */
void ca_init(ca_mode_t ca_mode, unsigned int screen_width,
             unsigned int screen_height, color_t *colors,
             unsigned int update_delay) {

  unsigned int border_width = 1;
  // initialize GL for non-custom modes
  gl_init(screen_width + 2 * border_width, screen_height + 2 * border_width,
          GL_DOUBLEBUFFER); // initialize frame buffer

  // init all pixels to background color (in both buffers)
  gl_clear(colors[0]);
  gl_swap_buffer();
  gl_clear(colors[0]);
  gl_swap_buffer();

  // init global struct
  ca.mode = ca_mode;
  ca.state_colors = colors;
  ca.bordered_height = screen_height + 2 * border_width;
  ca.bordered_width = screen_width + 2 * border_width;
  ca.height = screen_height;
  ca.width = screen_width;
  ca.padded_width = fb_get_pitch() / fb_get_depth();
  ca.update_ms = update_delay;

  // init file system
  ca_ffs_init();
}

/*
 * Function: save_state
 * --------------------------
 * Save the given `state` of the framebuffer to the file `fname`. Strip
 * padding before writing.
 *
 * Note: ca_ffs_init() must be called before hand. TODO: add a check? move this
 * to read_write_ca?
 */
void save_state(const char *fname, void *state) {

  // int n = recursive_scan(""); // start at root
  // printf("Scan found %d entries.\n\n", n);

  // TODO: choose unsigned int or color_t and be consistent
  // TODO: remove these failed options
  // color_t writebuf[] = (color_t *) ca.cur_state;
  // color_t *writebuf = (color_t *) ca.cur_state;

  // remove padding from buffer before writing
  unsigned int(*state_2d)[ca.padded_width] = state;
  unsigned int bytes = 4 * ca.width * ca.height;
  void *writebuf = malloc(bytes); // TODO: use the heap because of dynamic size?
                                  // TODO: best practice w void *?
  color_t(*writebuf_2d)[ca.width] = writebuf;
  for (int i = 0; i < ca.height; i++) {
    for (int j = 0; j < ca.width; j++) {
      writebuf_2d[i][j] = state_2d[i][j];
    }
  }
  write_preset(writebuf, bytes, fname);
  free(writebuf);
}

/*
 * Function: load_preset
 * --------------------------
 * Read the preset from the file `fname` into the given `state` of the
 * framebuffer. Write using the appropriate padding for the framebuffer.
 *
 * Note: ca_ffs_init() must be called before hand. TODO: add a check? move this
 * to read_write_ca?
 */
void load_preset(const char *fname, void *state) {
  // int n = recursive_scan(""); // start at root
  // printf("Scan found %d entries.\n\n", n);

  unsigned int bytes = 4 * ca.width * ca.height;
  color_t readbuf[bytes];
  read_preset(readbuf, bytes, fname);

  // copy into the next frame buffer, then swap and update
  color_t(*readbuf_2d)[ca.width] = &readbuf; // TODO: is this right?
  unsigned int(*state_2d)[ca.padded_width] = state;
  for (int c = 0; c < ca.height; c++) {
    for (int r = 0; r < ca.width; r++) {
      state_2d[r][c] = readbuf_2d[r][c];
    }
  }
}

/*
 * Function: count_neighbors_moore
 * --------------------------
 * This function returns the number of directly-adjacent or diagonal neighbors
 * of cell
 * (`r`, `c`) that have state (aka color) `state_to_count` in `state`.
 */
static unsigned int count_neighbors_moore(unsigned int r, unsigned int c,
                                          unsigned int state_to_count,
                                          void *state) {
  unsigned int(*state_2d)[ca.padded_width] = state;

  unsigned int target_count = 0;

  unsigned int next_col = (c == ca.width) ? 0 : c + 1;
  unsigned int prev_col = (c == 0) ? ca.width - 1 : c - 1;
  unsigned int next_row = (r == ca.height) ? 0 : r + 1;
  unsigned int prev_row = (r == 0) ? ca.height - 1 : r - 1;

  target_count += (state_2d[r][next_col] == state_to_count);
  target_count += (state_2d[prev_row][c] == state_to_count);
  target_count += (state_2d[r][prev_col] == state_to_count);
  target_count += (state_2d[next_row][c] == state_to_count);

  target_count += (state_2d[prev_row][prev_col] == state_to_count);
  target_count += (state_2d[prev_row][next_col] == state_to_count);
  target_count += (state_2d[next_row][prev_col] == state_to_count);
  target_count += (state_2d[next_row][next_col] == state_to_count);

  return target_count;
}

/*
 * Function: count_neighbors_von_neumann
 * --------------------------
 * This function returns the number of directly-adjacent neighbors of cell (`r`,
 * `c`) that have state (aka color) `state_to_count` in `state`.
 */
static unsigned int count_neighbors_von_neumann(unsigned int r, unsigned int c,
                                                unsigned int state_to_count,
                                                void *state) {
  unsigned int(*state_2d)[ca.padded_width] = state;

  unsigned int target_count = 0;

  target_count += (state_2d[r][(c + 1) % ca.width] == state_to_count);
  target_count += (state_2d[(r + 1) % ca.height][c] == state_to_count);
  target_count +=
      (state_2d[r][(c - 1 + ca.width) % ca.width] == state_to_count);
  target_count +=
      (state_2d[(r - 1 + ca.height) % ca.height][c] == state_to_count);

  return target_count;
}

/*
 * DEPRECATED !!! Use cpu_ww_update_state instead
 * Function: wireworld_update_pix
 * --------------------------
 * Handle pixel update for (`r`, `c`) in `state` for WireWorld.
 *
 * Reference: https://mathworld.wolfram.com/WireWorld.html
 *
 * TODO: move to its own module? move to draw_ca module?
 */
static unsigned int wireworld_update_pix(unsigned int r, unsigned int c,
                                         void *state) {
  unsigned int(*state_2d)[ca.padded_width] = state;
  unsigned int cell_state = state_2d[r][c];

  if (cell_state == ca.state_colors[1]) {
    // electron head always turns into an electron tail
    return ca.state_colors[2];
  } else if (cell_state == ca.state_colors[2]) {
    // electron tail always turns into wire
    return ca.state_colors[3];
  } else if (cell_state == ca.state_colors[3]) {
    // wire remains wire unless u is 1 or 2 (becomes an electron head)
    unsigned int live_neighbors =
        count_neighbors_moore(r, c, ca.state_colors[1], state);
    if (live_neighbors == 1 || live_neighbors == 2) {
      return ca.state_colors[1];
    }
  }
  return cell_state;
}

/*
 * Function: cpu_ww_update_state
 * --------------------------
 * Takes in a prev state and a new state and updates each pixel individually
 */
static void cpu_ww_update_state(void *prev, void *next) {
  unsigned int(*next_state_2d)[ca.padded_width] = next;
  unsigned int(*prev_state_2d)[ca.padded_width] = (void *)prev;

  // update each pixel
  for (int c = 0; c < ca.height; c++) {
    for (int r = 0; r < ca.width; r++) {

      unsigned int cell_state = prev_state_2d[r][c];
      unsigned int new_state = cell_state;

      if (cell_state == ca.state_colors[1]) {
        // electron head always turns into an electron tail
        new_state = ca.state_colors[2];
      } else if (cell_state == ca.state_colors[2]) {
        // electron tail always turns into wire
        new_state = ca.state_colors[3];
      } else if (cell_state == ca.state_colors[3]) {
        // wire remains wire unless u is 1 or 2 (becomes an electron head)
        unsigned int live_neighbors =
            count_neighbors_moore(r, c, ca.state_colors[1], prev);
        if (live_neighbors == 1 || live_neighbors == 2) {
          new_state = ca.state_colors[1];
        }
      }

      // write pixel in next
      next_state_2d[r][c] = new_state;
    }
  }
}

/*
 * Function: game_of_life_update_pix
 * --------------------------
 * This function updates the state of the cell at coordinate (`r`, `c`).
 * By observing `state`, it considers its current state and the state of its
 * neighbors. It returns the new state of the cell.
 *
 * `ca.state_colors[1]` and `ca.state_colors[0]` respectively specify the colors
 * of live and dead cells.
 *
 * TODO: move to its own module? move to draw_ca module?
 */
static unsigned int game_of_life_update_pix(unsigned int r, unsigned int c,
                                            void *state) {
  unsigned int(*state_2d)[ca.padded_width] = state;
  unsigned int cell_state = state_2d[r][c];

  unsigned int live_neighbors =
      count_neighbors_moore(r, c, ca.state_colors[1], state);

  if (cell_state == ca.state_colors[1]) {
    if (live_neighbors == 2 || live_neighbors == 3) {
      // printf("row %d, col %d, neighs %d\n", r, c, live_neighbors);
      return ca.state_colors[1]; // live cell w/ 2 or 3 live neighbours survives
    } else {
      return ca.state_colors[0]; // otherwise, it dies
    }
  } else if (cell_state == ca.state_colors[0]) {
    if (live_neighbors == 3) {
      // printf("row %d, col %d, neighs %d\n", r, c, live_neighbors);
      return ca.state_colors[1]; // dead cell w/ 3 live neighbours becomes a
                                 // live cell
    } else {
      return ca.state_colors[0]; // otherwise, it remains dead
    }
  } else {
    return cell_state; // TODO: catch exception
  }
}

/*
 * Function: update_state
 * --------------------------
 * This function updates the state of the cellular automata by one step using
 * the appropriate update function (specified by ca.mode).
 */
static void update_state(void *prev, void *next) {
  ca_modes[ca.mode].fn(prev, next);
}

static void gpu_gol_update_state(void *prev, void *next) {
  unsigned int *cur_state = prev;
  unsigned int *next_state = (unsigned int *)next;

  unsigned int number_of_uniforms = 7;
  unsigned program[] = {
#include "../tests/life_driver.c"
  };

  qpu_init();

  // Uniforms
  // 1. Number of rows
  // 2. Number of columns (must be multiple of 16)
  // 3. Frame buffer's padded_width
  // 4. Off color
  // 5. On color
  // 6. Prev state -- Current FB pointer (to read to)
  // 7. Next state -- Next FB pointer (to write to)
  unsigned uniforms[] = {
      (unsigned)ca.height,          (unsigned)ca.width,
      (unsigned)ca.padded_width,    (unsigned)ca.state_colors[0],
      (unsigned)ca.state_colors[1], (unsigned)cur_state,
      (unsigned)next_state};
  // printf("gpu updating state for sliding window\n");
  qpu_run(program, SIZE(program), uniforms, number_of_uniforms);
  // printf("--\n");
  while (qpu_request_count() != qpu_complete_count()) {
  }
  assert(qpu_request_count() == qpu_complete_count());
  assert(qpu_complete_count() == 1);
}

static void gpu_gol_update_state_cdriver(unsigned int *prev, void *next) {
  unsigned int *cur_state = prev;
  unsigned int *next_state = (unsigned int *)next;

  unsigned int number_of_uniforms = 7;
  unsigned program[] = {
#include "../tests/fb_life.c"
  };

  for (int r = 0; r < ca.height; r++) {
    for (int c = 0; c < ca.width; c += 16) {
      // printf("(%d, %d)\n", r, c);
      qpu_init();

      unsigned uniforms[] = {
          // get pointers to a cell at (1, 1) aka unpadded (0, 0) and its 3 left
          // neighbors
          (unsigned)ca.state_colors[0], (unsigned)ca.state_colors[1],
          (unsigned)(cur_state + ca.padded_width * (r + 1) + (c + 1)),
          (unsigned)(cur_state + ca.padded_width * (r) + (c)),
          (unsigned)(cur_state + ca.padded_width * (r + 1) + (c)),
          (unsigned)(cur_state + ca.padded_width * (r + 2) + (c)),
          // get update address
          (unsigned)(next_state + ca.padded_width * (r + 1) + (c + 1))};
      // printf("gpu updating state for sliding window\n");
      qpu_run(program, SIZE(program), uniforms, number_of_uniforms);
      // printf("--\n");
      assert(qpu_request_count() == qpu_complete_count());
      assert(qpu_complete_count() == 1);
    }
  }
}

/*
 * Function: ca_create_and_load_preset
 * --------------------------
 * This function creates a preset using the given function `make_preset`.
 * If `save_preset`, the preset is saved in file `fname`.
 *
 * It must be preceded by a call to `ca_init()`.
 *
 * If the given `fname` exists, it will be overwritten.
 */
void ca_create_and_load_preset(const char *fname, preset_fn_t make_preset,
                               unsigned int save_to_sd) {
  ca.cur_state = fb_get_draw_buffer();
  make_preset(ca.width, ca.height, ca.padded_width, ca.cur_state,
              ca.state_colors);
  if (save_to_sd) {
    save_state(fname, ca.cur_state);
  }
}

/*
 * Function: ca_run
 * --------------------------
 * This function runs a cellular automata loop that never returns.
 *
 * It must be preceded by a call to `ca_init()` because that contains
 * `ca_ffs_init()`.
 *
 * It should also be preceded by a function that loads a preset.
 */
void ca_run(unsigned int use_time_limit, unsigned int ticks_to_run,
            unsigned main_button) {
  // printf("RUNNING!");
  // display the initial state
  ca.cur_state = fb_get_draw_buffer();
  gl_swap_buffer();

  unsigned int start = timer_get_ticks();
  unsigned int total_updates = 0;
  while ((!use_time_limit) || (timer_get_ticks() < ticks_to_run + start)) {
    timer_delay_ms(ca.update_ms);

    // retrieve buffer for the next state
    ca.next_state = fb_get_draw_buffer();

    // gpu_gol_update_state(ca.cur_state, ca.next_state);
    // printf("ticks per update: %d\n", timer_get_ticks() - prev_ticks);
    update_state(ca.cur_state, ca.next_state);

    // // show the buffer for ca.next_state
    gl_swap_buffer();
    // make ca.next_state the ca.cur_state
    ca.cur_state = ca.next_state;

    total_updates++;

    // printf("loop status: %d, %d, %d\n", use_time_limit, timer_get_ticks(),
    // ticks_to_run + start);

    // end ca if main button was held down
    if (check_button_dequeue(main_button) == BUTTON_HOLD) {
      break;
    }
  }

  printf("total updates: %d\n", total_updates);
}
