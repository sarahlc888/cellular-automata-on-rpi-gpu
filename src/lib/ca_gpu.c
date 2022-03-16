/* Sarah Chen
 * 03/08/2022
 * Code for CS107E Final Project
 *
 * This module handles representation and simulation of cellular automata.
 *
 * Supported automata:
 * - The Game of Life (default on GPU, CPU is also supported)
 * - WireWorld (on CPU)
 *
 * The grid state is stored directly in the frame buffers, and states are 
 * implicitly represented as colors. (Since the display is double-buffered, 
 * one buffer is the previous state that governs the update for the next 
 * buffer/state.)
 *
 * The Game of Life is intended for an infinite grid, but this module handles
 * the borders of the grid using a toroidal approach (for the CPU) or by 
 * maintaining a 1-unit wide border of dead cells (for the GPU).
 * 
 * A number of functions in this library such as `count_neighbors_von_neumann()` 
 * and `cpu_life_update_state()` and features like the mode title vs. styled title 
 * are not explicitly used (leading to compilation warnings), but we keep them 
 * here for extensibility reasons.
 */

#include "../../include/ca_gpu.h"
#include "../../include/button.h"
#include "../../include/draw_ca.h"
#include "../../include/qpu.h"
#include "../../include/read_write_ca.h"
#include "assert.h"
#include "fb.h"
#include "gl.h"
#include "malloc.h"
#include "printf.h"
#include "timer.h"

#define SIZE(x) sizeof(x) / sizeof(x[0])

// struct for information about the current cellular automata simulation
typedef struct {
  unsigned int mode;            // which cellular automata to run

  unsigned int height;          // height of the physical screen
  unsigned int width;           // width of the physical screen
  unsigned int bordered_height; // height of the physical screen
  unsigned int bordered_width;  // width of the physical screen
  unsigned int padded_width;    // width of the physical screen plus padding
  color_t *state_colors;        // colors for various states

  unsigned int update_ms;       // ms between state updates

  unsigned int *cur_state;      // current framebuffer
  unsigned int *next_state;     // next framebuffer

} ca_config_t;

static ca_config_t ca;
const unsigned int border_width = 1; // pixel width of border for zero-padding

// CA mode table links titles, styled titles, and update functions
static void gpu_life_update_state(void *prev, void *next);
static void cpu_ww_update_state(void *prev, void *next);
static void cpu_life_update_state(void *prev, void *next);
static const ca_option_t ca_modes[] = {
    {"life", "Conway", gpu_life_update_state},
    {"wireworld", "WireWorld!", cpu_ww_update_state},
};

/*
 * Function: ca_init
 * --------------------------
 * Initialize a cellular automaton simulation with mode `ca_mode_t` to specify
 * which automaton to use and whether or not to use a custom start state.
 * 
 * The grid will be of dimensions `screen_width` by `screen_height`. (Note 
 * that the requested framebuffer will be wider by 2 pixels in each direction
 * due to padding.)
 * 
 * IMPORTANT: `screen_width` must be a multiple of 16 to support the sliding
 * window vectorization approach for game of life.
 * 
 * The CA will use `colors`, a client-specified color array. The first color is 
 * the background color, and the remaining colors play roles specified by the
 * CA. (For Life, colors[1] = alive state. For WireWorld, colors[1] = electron 
 * head, colors[2] = tail, colors[3] = wire.) 
 * 
 * The `update_delay` in milliseconds controls the delay between screen refreshes. 
 * Set it as 0 if no delay is desired.
 */
void ca_init(ca_mode_t ca_mode, unsigned int screen_width,
             unsigned int screen_height, color_t *colors,
             unsigned int update_delay) {
  // require that screen width is a multiple of 16
  assert(screen_width % 16 == 0);

  // initialize GL for non-custom modes
  gl_init(screen_width + 2 * border_width, screen_height + 2 * border_width,
          GL_DOUBLEBUFFER); // initialize frame buffer

  // initialize all pixels to background color (in both buffers)
  gl_clear(colors[0]);
  gl_swap_buffer();
  gl_clear(colors[0]);
  gl_swap_buffer();

  // initialize global struct
  ca.mode = ca_mode;
  ca.state_colors = colors;
  ca.bordered_height = screen_height + 2 * border_width;
  ca.bordered_width = screen_width + 2 * border_width;
  ca.height = screen_height;
  ca.width = screen_width;
  ca.padded_width = fb_get_pitch() / fb_get_depth();
  ca.update_ms = update_delay;

  // initialize file system
  ca_ffs_init();
}

/*
 * Function: save_state
 * --------------------------
 * Save the given `state` of the framebuffer to the file `fname`. Strip
 * padding before writing, but include the border
 *
 * Note: `ca_ffs_init()` must be called before this function, and that should
 * be done in `ca_init()`
 */
void save_state(const char *fname, void *state) {
  // remove padding from buffer before writing
  unsigned int(*state_2d)[ca.padded_width] = state;
  unsigned int bytes = 4 * ca.bordered_width * ca.bordered_height;
  void *writebuf = malloc(bytes); 
  color_t(*writebuf_2d)[ca.bordered_width] = writebuf;
  for (int i = 0; i < ca.bordered_height; i++) {
    for (int j = 0; j < ca.bordered_width; j++) {
      writebuf_2d[i][j] = state_2d[i][j];
    }
  }

  // output the data
  write_preset(writebuf, bytes, fname);
  free(writebuf);
}

/*
 * Function: load_preset
 * --------------------------
 * Read the preset from the file `fname` into the given `state` of the
 * framebuffer. Write using the appropriate padding for the framebuffer.
 *
 * Note: `ca_ffs_init()` must be called before this function, and that should
 * be done in `ca_init()`
 */
void load_preset(const char *fname, void *state) {
  unsigned int bytes = 4 * ca.bordered_width * ca.bordered_height;
  color_t readbuf[bytes];
  read_preset(readbuf, bytes, fname);

  // copy into the next frame buffer, then swap and update
  color_t(*readbuf_2d)[ca.bordered_width] = &readbuf; 
  unsigned int(*state_2d)[ca.padded_width] = state;
  for (int c = 0; c < ca.bordered_height; c++) {
    for (int r = 0; r < ca.bordered_width; r++) {
      state_2d[r][c] = readbuf_2d[r][c];
    }
  }
}

/*
 * Function: count_neighbors_moore
 * --------------------------
 * This function returns the number of directly-adjacent or diagonal neighbors
 * of cell (`r`, `c`) that have state (aka color) `state_to_count` in `state`.
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
 * Function: cpu_ww_update_state
 * --------------------------
 * Takes in a prev state and a new state and updates each pixel individually
 * Handle CPU pixel update for (`r`, `c`) in `state` for WireWorld.
 *
 * Rules found at: https://mathworld.wolfram.com/WireWorld.html
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
 * Function: cpu_life_update_state
 * --------------------------
 * This function updates the state of the cell at coordinate (`r`, `c`) using 
 * the CPU. By observing `state`, it considers its current state and the state 
 * of its neighbors. It returns the new state of the cell.
 *
 * `ca.state_colors[1]` and `ca.state_colors[0]` respectively specify the colors
 * of live and dead cells.
 */
static void cpu_life_update_state(void *prev, void *next) {
  unsigned int(*next_state_2d)[ca.padded_width] = next;
  unsigned int(*prev_state_2d)[ca.padded_width] = (void *)prev;

  // update each pixel
  for (int c = 0; c < ca.height; c++) {
    for (int r = 0; r < ca.width; r++) {
      unsigned int cell_state = prev_state_2d[r][c];

      unsigned int live_neighbors =
          count_neighbors_moore(r, c, ca.state_colors[1], prev);

      // assert that the cell is one of the expected colors
      assert(cell_state == ca.state_colors[0] || cell_state == ca.state_colors[1]);

      if (cell_state == ca.state_colors[1]) {
        if (live_neighbors == 2 || live_neighbors == 3) {
          next_state_2d[r][c] = ca.state_colors[1]; // live cell w/ 2 or 3 live neighbours survives
        } else {
          next_state_2d[r][c] = ca.state_colors[0]; // otherwise, it dies
        }
      } else if (cell_state == ca.state_colors[0]) {
        if (live_neighbors == 3) {
          next_state_2d[r][c] = ca.state_colors[1]; // dead cell w/ 3 live neighbours becomes a
                                    // live cell
        } else {
          next_state_2d[r][c] = ca.state_colors[0]; // otherwise, it remains dead
        }
      } 
    }
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

/*
 * Function: gpu_life_update_state
 * --------------------------
 * This function updates the state of the Game of Life by one step using
 * the GPU. The update logic for each refresh is fully encoded in the
 * included program, `src/qasm/life_driver.c`.
 * 
 * The program runs a sliding window of size 16 over the current fb grid and
 * (1) Computes neighbor vectors for the 8 neighbors of cells in the sliding window
 * (2) Determines the number of alive neighbors for each cell via vector addition
 * (3) Writes the new state of the cell to the next fb pointer
 * 
 * To do so, the GPU must read and write from the main memory, which is 
 * managed using direct memory access (DMA) between the main memory and 
 * the GPU's Vertex Pipe Memory (VPM). It then reads the stored vectors 
 * into GPU registers, does computation, and then writes the results back
 * into the framebuffer.
 */
static void gpu_life_update_state(void *prev, void *next) {
  unsigned int *cur_state = prev;
  unsigned int *next_state = (unsigned int *) next;

  unsigned program[] = {
    #include "../qasm/life_driver.c"
  };

  qpu_init();

  assert(ca.width % 16 == 0); // Require grid width to support sliding window of 16 cells
  unsigned uniforms[] = {
      (unsigned) ca.height,           // number of rows in CA grid (not including border)
      (unsigned) ca.width,            // number of columns in CA grid (not including border)
      (unsigned) ca.padded_width,     // frame buffer's padded_width
      (unsigned) ca.state_colors[0],  // off cell color
      (unsigned) ca.state_colors[1],  // on cell color
      (unsigned) cur_state,           // previous state (the current fb pointer, to read from)
      (unsigned) next_state};         // next state (next fb pointer, to write to)

  // send the program to the GPU and wait for completion
  qpu_run(program, SIZE(program), uniforms, SIZE(uniforms)); 
  while (qpu_request_count() != qpu_complete_count()) {}

  assert(qpu_request_count() == qpu_complete_count());
  assert(qpu_complete_count() == 1);

}

/*
 * Function: gpu_gol_update_state_cdriver
 * --------------------------
 * This function updates the state of the Game of Life by one step using
 * the GPU. The update logic for each refresh requires a C driver to control
 * the sliding window, and the GPU is re-initialized for every sliding window. 
 * It is kept here for archival purposes and to enable a speed comparison.
 * 
 * It is slower than CPU updates.
 */
static void gpu_gol_update_state_cdriver(unsigned int *prev, void *next) {
  unsigned int *cur_state = prev;
  unsigned int *next_state = (unsigned int *)next;

  unsigned program[] = {
    #include "../qasm/fb_life.c"
  };

  for (int r = 0; r < ca.height; r++) {
    for (int c = 0; c < ca.width; c += 16) {
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
      qpu_run(program, SIZE(program), uniforms, SIZE(uniforms));
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
            unsigned main_button, unsigned verbose) {
  // display the initial state
  ca.cur_state = fb_get_draw_buffer();
  gl_swap_buffer();

  unsigned int start = timer_get_ticks();
  unsigned int prev_ticks = timer_get_ticks();
  unsigned int total_updates = 0;
  unsigned int total_ticks = 0;
  while ((!use_time_limit) || (timer_get_ticks() < ticks_to_run + start)) {
    timer_delay_ms(ca.update_ms);

    // retrieve buffer for the next state
    ca.next_state = fb_get_draw_buffer();

    update_state(ca.cur_state, ca.next_state);
    if (verbose) {
      printf("ticks per update: %d\n", timer_get_ticks() - prev_ticks);
    }

    // show the buffer for ca.next_state
    gl_swap_buffer();

    // make ca.next_state the ca.cur_state
    ca.cur_state = ca.next_state;

    total_ticks += (timer_get_ticks() - prev_ticks); // TODO: catch overflow?
    prev_ticks = timer_get_ticks();
    total_updates++;

    // end ca if main button was held down
    if (check_button_dequeue(main_button) == BUTTON_HOLD) {
      break;
    }
  }

  if (verbose) {
    printf("total updates: %d\n", total_updates);
  }
}
