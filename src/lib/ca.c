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
#include "malloc.h"
#include "../../include/ca.h"
#include "../../include/read_write_ca.h"
#include "../../include/draw_ca.h"

typedef struct {
    unsigned int mode;          // which cellular automata to run

    unsigned int height;        // height of the physical screen
    unsigned int width;         // width of the physical screen
    unsigned int padded_width;  // width of the physical screen plus padding
    color_t* state_colors;      // colors for various states

    unsigned int update_ms;   // ms between state updates

    // pointers to the framebuffer
    unsigned int *cur_state;
    unsigned int *next_state;

} ca_config_t;


static volatile ca_config_t ca; // TODO: why volatile?

static unsigned int game_of_life_update_pix(unsigned int r, unsigned int c, void *state);
static unsigned int wireworld_update_pix(unsigned int r, unsigned int c, void *state);
static const ca_option_t ca_modes[] = {
    {"life",        "Conway",       game_of_life_update_pix},
    {"wireworld",   "WireWorld!",   wireworld_update_pix},
};


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
    // TODO: specify mode by string?

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

    // init file system
    ca_ffs_init();

}

// save the current state of the framebuffer
void save_state(const char *fname, void *state)
{

    int n = recursive_scan(""); // start at root
    printf("Scan found %d entries.\n\n", n);

    // color_t writebuf[] = (color_t *) ca.cur_state; // TODO: choose unsigned int or color_t and be consistent
    // TODO: choose unsigned int or color_t and be consistent
    // color_t *writebuf = (color_t *) ca.cur_state; 

    // TODO: remove padding from buffer before writing
    unsigned int (*state_2d)[ca.padded_width] = state;
    unsigned int bytes = 4 * ca.width * ca.height;
    void *writebuf = malloc(bytes); // TODO: use the heap because of dynamic size?
    // TODO: best practice w void *?
    color_t (*writebuf_2d)[ca.width] = writebuf;

    for (int i = 0; i < ca.height; i++) {
        for (int j = 0; j < ca.width; j++) {
            writebuf_2d[i][j] = state_2d[i][j];
        }
    }

    write_preset(writebuf, bytes, fname);

    free(writebuf);
}

// read the preset into the cur_state
// TODO: handle padding (don't write it but handle its lack while reading it)
// TODO: call ca_ffs_init before
void load_preset(const char *fname, void *state)
{
    int n = recursive_scan(""); // start at root
    printf("Scan found %d entries.\n\n", n);

    unsigned int bytes = 4 * ca.width * ca.height;
    color_t readbuf[bytes];
    read_preset(readbuf, bytes, fname);

    // copy into the next frame buffer, then swap and update
    color_t (*readbuf_2d)[ca.width] = &readbuf; // TODO: is this right?
    unsigned int (*state_2d)[ca.padded_width] = state; 
    for (int c = 0; c < ca.height; c++) {
        for (int r = 0; r < ca.width; r++) {
            state_2d[r][c] = readbuf_2d[r][c];
        }
    }

    printf("Finished reading\n");
    // // show the buffer for ca.next_state
    // gl_swap_buffer();
    // // make ca.next_state the ca.cur_state
    // ca.cur_state = ca.next_state;
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
    // ww_draw_or_gate(10, 10, 4, 1, 0, state, ca.padded_width, ca.state_colors);
    // return; 

    for (int i = 5; i < ca.width - 5; i += 5) {
        if (ca.mode == 1) {
            ww_draw_row_wire(i, 10, ca.width - 20, state, ca.padded_width, ca.state_colors);
        }
        if (ca.mode == 0) {
            for (int j = 5; j < ca.height - 5; j += 5) {
                life_draw_osc(i, j, state, ca.padded_width, ca.state_colors);
            }
        }
    }
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

// https://mathworld.wolfram.com/WireWorld.html
static unsigned int wireworld_update_pix(unsigned int r, unsigned int c, void *state) 
{
    unsigned int (*state_2d)[ca.padded_width] = state;
    unsigned int cell_state = state_2d[r][c];

    if (cell_state == ca.state_colors[1]) {
        // electron head always turns into an electron tail
        return ca.state_colors[2]; 
    } else if (cell_state == ca.state_colors[2]) {
        // electron tail always turns into wire
        return ca.state_colors[3]; 
    } else if (cell_state == ca.state_colors[3]) {
        // wire remains wire unless u is 1 or 2 (becomes an electron head)
        unsigned int live_neighbors = count_neighbors_moore(r, c, ca.state_colors[1], state);
        if (live_neighbors == 1 || live_neighbors == 2) {
            return ca.state_colors[1]; 
        }
    }
    return cell_state;
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
 * TODO: I should probably decompose further so that the states are not determined
 * by the colors. I should probably malloc rather than keeping everything implicit
 * in the framebuffers.
 */
static unsigned int game_of_life_update_pix(unsigned int r, unsigned int c, void *state) 
{
    unsigned int (*state_2d)[ca.padded_width] = state;
    unsigned int cell_state = state_2d[r][c];
    unsigned int live_neighbors = count_neighbors_moore(r, c, ca.state_colors[1], state);


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
            return ca.state_colors[1]; // dead cell w/ 3 live neighbours becomes a live cell
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
 * This function updates the state of the cellular automata by one step.
 */
static void update_state(unsigned int *prev, void *next)
{
    unsigned int (*state_2d)[ca.padded_width] = next;

    // update each pixel
    for (int c = 0; c < ca.height; c++) {
        for (int r = 0; r < ca.width; r++) {

            // read state from prev
            // TODO: move this outside the loop?
            unsigned int new_state = ca_modes[ca.mode].fn(r, c, prev);
            // write pixel in next
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
    
    // load and display the initial state
    ca.cur_state = fb_get_draw_buffer();
    load_preset("/presets/test_preset.rgba", ca.cur_state);
    // create_initial_state(ca.cur_state, ca.state_colors[1]); 
    // save_state("/presets/test_preset.rgba", ca.cur_state); // The code used to create the preset

    // printf("inital state is ca.cur_state %p\n", ca.cur_state);
    gl_swap_buffer(); 

    while (1)
    {
        timer_delay_ms(ca.update_ms);

        // retrieve buffer for the next state
        ca.next_state = fb_get_draw_buffer();
        printf("updating. ca.cur_state %p, ca.next_state %p\n", ca.cur_state, ca.next_state);
        update_state(ca.cur_state, ca.next_state);

        // show the buffer for ca.next_state
        gl_swap_buffer();
        // make ca.next_state the ca.cur_state
        ca.cur_state = ca.next_state;
    }
}
