/*
 * Code for CS107E Final Project
 *
 * Sarah Chen
 * 
 * Module currently supports a basic game of life cellular automata.
 * 
 * TODO: could support rule tables 
 * https://github.com/GollyGang/ruletablerepository/blob/gh-pages/src/read_ruletable.cpp
 * http://www.mirekw.com/ca/rullex_rtab.html 
 * 
 * TODO: borders -- need to make this bigger than the space ? to simulate infinity?
 * 
 * Design choice was: don't malloc the state, just implicitly store the state in the 
 * frame buffers. Consider one and edit the other, making states and colors identical,
 * to avoid having 2 other prev/next buffers as well.
 * 
 * TODO: Langston's loop. 
 * - support multiple states using the colors array rather than separate on/off variables.
 * - support more granular neighbor information (which neighbors are on)
 * - operate based on rule table to simplify implementation?
 */

#include "timer.h"
#include "fb.h"
#include "gl.h"
#include "printf.h"

typedef struct {
    unsigned int mode;          // which cellular automata to run

    unsigned int height;        // height of the physical screen
    unsigned int width;         // width of the physical screen
    unsigned int padded_width;  // width of the physical screen plus padding
    color_t* state_colors;      // colors for various states

    unsigned int update_ms;   // ms between state updates
} ca_config_t;


static volatile ca_config_t ca; // TODO: why volatile?

/*
 * Function: ca_init
 * --------------------------
 * The first color passed in `colors` is presumed to be the background color.
 */
void ca_init(unsigned int ca_mode, 
    unsigned int screen_width, unsigned int screen_height, int num_states,
    color_t* colors,
    unsigned int update_delay)
{
    gl_init(screen_width, screen_height, GL_DOUBLEBUFFER); // initialize frame buffer

    // init all pixels to background color (in both buffers)
    gl_clear(colors[0]);
    gl_swap_buffer();
    gl_clear(colors[0]);
    gl_swap_buffer();

    // init global struct
    ca.mode = ca_mode;
    ca.state_colors = colors;
    ca.width = screen_width;
    ca.padded_width = fb_get_pitch() / fb_get_depth(); 
    ca.height = screen_height;
    ca.update_ms = update_delay; 

}

void make_glider(int r, int c, void *state, color_t on_state_color)
{
    unsigned int (*state_2d)[ca.padded_width] = state;

    state_2d[r][c - 1] = on_state_color;
    state_2d[r][c] = on_state_color;
    state_2d[r][c + 1] = on_state_color;
}

/*
 * Function: create_initial_state
 * --------------------------
 * This function populates `state` with an initial test state.
 * 
 * TODO: read initial state from a file.
 */
void create_initial_state(void *state, color_t on_state_color)
{
    // unsigned int (*state_2d)[ca.padded_width] = state;

    // TODO: make this not random
    for (int i = 5; i < ca.width - 5; i += 5) {
        for (int j = 5; j < ca.height - 5; j += 5) {
            make_glider(i, j, state, on_state_color);
        }
    }

    make_glider(10, 10, state, on_state_color);

    // unsigned int (*state_2d)[ca.padded_width] = state;

    // int r = 10;
    // int c = 20;
    // state_2d[r][c - 1] = on_state_color;
    // state_2d[r][c] = on_state_color;
    // state_2d[r][c + 1] = on_state_color;
}

/*
 * Function: count_neighbors_moore
 * --------------------------
 * This function returns the number of directly-adjacent or diagonal neighbors of cell 
 * (`r`, `c`) that have state (aka color) `state_to_count` in `state`.
 */
static unsigned int count_neighbors_moore(unsigned int r, unsigned int c, unsigned int state_to_count, void *state)
{
    unsigned int (*state_2d)[ca.padded_width] = state;

    unsigned int target_count = 0;

    target_count += (state_2d[r][c + 1] == state_to_count);
    target_count += (state_2d[r + 1][c] == state_to_count);
    target_count += (state_2d[r][c - 1] == state_to_count);
    target_count += (state_2d[r - 1][c] == state_to_count);

    target_count += (state_2d[r - 1][c - 1] == state_to_count);
    target_count += (state_2d[r - 1][c + 1] == state_to_count);
    target_count += (state_2d[r + 1][c - 1] == state_to_count);
    target_count += (state_2d[r + 1][c + 1] == state_to_count);

    return target_count;
}

/*
 * Function: count_neighbors_von_neumann
 * --------------------------
 * This function returns the number of directly-adjacent neighbors of cell (`r`, `c`) 
 * that have state (aka color) `state_to_count` in `state`.
 */
static unsigned int count_neighbors_von_neumann(unsigned int r, unsigned int c, unsigned int state_to_count, void *state)
{
    unsigned int (*state_2d)[ca.padded_width] = state;

    unsigned int target_count = 0;

    target_count += (state_2d[r][c + 1] == state_to_count);
    target_count += (state_2d[r + 1][c] == state_to_count);
    target_count += (state_2d[r][c - 1] == state_to_count);
    target_count += (state_2d[r - 1][c] == state_to_count);

    return target_count;
}

/*
 * Function: game_of_life_update_pix
 * --------------------------
 * This function updates the state of the cell at coordinate (`r`, `c`).
 * By observing `state`, it considers its current state and the state of its 
 * neighbors. It returns the new state of the cell.
 * 
 * The `live_state` and `dead_state` specify the colors of live and dead cells.
 * 
 * TODO: I should probably decompose further so that the states are not determined
 * by the colors. I should probably malloc rather than keeping everything implicit
 * in the framebuffers.
 */
static unsigned int game_of_life_update_pix(unsigned int r, unsigned int c, void *state,
    color_t live_state, color_t dead_state) 
{
    unsigned int (*state_2d)[ca.padded_width] = state;
    unsigned int cell_state = state_2d[r][c];
    unsigned int live_neighbors = count_neighbors_moore(r, c, live_state, state);


    if (cell_state == live_state) {
        if (live_neighbors == 2 || live_neighbors == 3) {
            // printf("row %d, col %d, neighs %d\n", r, c, live_neighbors);
            return live_state; // live cell w/ 2 or 3 live neighbours survives
        } else {
            return dead_state; // otherwise, it dies
        }
    } else if (cell_state == dead_state) {
        if (live_neighbors == 3) {
            // printf("row %d, col %d, neighs %d\n", r, c, live_neighbors);
            return live_state; // dead cell w/ 3 live neighbours becomes a live cell
        } else {
            return dead_state; // otherwise, it remains dead
        }
    } else {
        return cell_state; // TODO: catch exception
    }
}

/*
 * Function: update_state
 * --------------------------
 * This function updates the state of the cellular automata by one step.
 */
static void update_state(unsigned int *prev, void *next)
{
    // update each pixel
    for (int c = 0; c < ca.height; c++) {
        for (int r = 0; r < ca.width; r++) {

            // read state from prev
            // TODO: update the state based on ca.mode rather than defaulting to the game of life
            // TODO: should i have separate update functions based on rule? 
            unsigned int new_state = game_of_life_update_pix(r, c, prev, ca.state_colors[1], ca.state_colors[0]);
            // write pixel in next
            unsigned int (*state_2d)[ca.padded_width] = next;
            state_2d[r][c] = new_state;
        }
    }

}

/*
 * Function: ca_run
 * --------------------------
 * This function runs a cellular automata loop that never returns.
 * 
 * It must be preceded by a call to `ca_init()`.
 */
void ca_run(void)
{
    unsigned int *cur_state;
    unsigned int *next_state;

    // load and display the initial state
    cur_state = fb_get_draw_buffer();
    create_initial_state(cur_state, ca.state_colors[1]); // TODO: implement this
    // printf("inital state is cur_state %p\n", cur_state);
    gl_swap_buffer(); 

    while (1)
    {
        timer_delay_ms(ca.update_ms);

        // retrieve buffer for the next state
        next_state = fb_get_draw_buffer();
        printf("updating. cur_state %p, next_state %p\n", cur_state, next_state);
        update_state(cur_state, next_state);

        // show the buffer and make next_state the cur_state
        gl_swap_buffer();
        cur_state = next_state;
    }
}
