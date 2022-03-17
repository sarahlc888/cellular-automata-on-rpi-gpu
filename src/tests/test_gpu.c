/* Sarah Chen
 * 03/14/2022
 * Code for CS107E Final Project
 * 
 * This module tests various programs on the Raspberry Pi GPU, starting with
 * a simple program to add two numbers (helloworld). Further tests work with
 * manipulating vectors (add_vectors), working in a 2D grid to support Life
 * (toy_life), writing/reading to/from the framebuffer (write_fb), a fully
 * vectorized sliding-window implementation of Life in the framebuffer (fb_life), 
 * and a vectorized Life that doesn't require a C driver (fb_life_nodriver).
 * 
 * The corresponding .qasm and .c files can be found in `gpu_examples/` and `src/qasm`
 * 
 * The original demo was taken directly from 
 * https://rpiplayground.wordpress.com/2014/05/03/hacking-the-gpu-for-fun-and-profit-pt-1/,
 * and the rest were created by adding features on top of it.
 */

#include "assert.h"
#include "printf.h"
#include "uart.h"
#include "malloc.h"
#include "interrupts.h"
#include "timer.h"
#include "strings.h"
#include "../../include/mailbox_functions.h"
#include "../../include/qpu.h"
#include "../../include/randomHardware.h"
#include "gl.h"

#define PHYS_TO_BUS(x) ((x)|0xC0000000)
#define SIZE(x) sizeof(x)/sizeof(x[0])

/*
 * Function: run_helloworld
 * --------------------------
 * Run an adapted helloworld qasm demo from 
 * https://rpiplayground.wordpress.com/2014/05/03/hacking-the-gpu-for-fun-and-profit-pt-1/
 * in a bare metal setting. 
 * 
 * It reads in a number, adds a constant, and writes the result to a designated 
 * address in main memory. Since the QPU works on vectors of size 16, it also loads
 * immediates into vectors and outputs the result as a 16-length vector too.
 * 
 * It adds 101 and 0x1019, which should yield 4761.
 */
void run_helloworld(void)
{
    qpu_init();

    // load the program, which should be assembled using vc4asm via
    // `vc4asm -i vc4.qinc -C ../../gpu_examples/helloworld.c ../../gpu_examples/helloworld.qasm`
    unsigned program[] = {
        #include "../../gpu_examples/helloworld.c"   
    };
    
    // allocate and lock GPU memory to hold the result 
    unsigned result_ptr = qpu_malloc(16);

    // create an array of uniforms
    unsigned uniforms[] = {101, result_ptr};

    // send the program to the GPU
    qpu_run(program, SIZE(program), uniforms, SIZE(uniforms));
    
    timer_delay(1); // allot time for it to run

    // should both output 1
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    printf("Result: 0x%x\n", *(volatile unsigned *) result_ptr );

    printf("Result vector: ");
    for (int j=0; j < 16; j++) {
        printf("0x%x ", *((unsigned int*)(result_ptr + 4 * j)));
    }
    printf("\n");
    qpu_free(result_ptr);
}

/*
 * Function: run_add_vectors
 * --------------------------
 * Run a demo to add four vectors together. The assembly program
 * - reads the addresses of the vectors from uniforms
 * - loads vectors from main memory into the VPM
 * - read vectors from the VPM into QPU registers
 * - computes sum
 * - writes vectors from the QPU registers into the VPM
 * - stores the vectors from the VPM into main memory
 * 
 * The result should be 5, 7, ..., 35
 */
void run_add_vectors(void)
{
    qpu_init();

    // load the program, which should be assembled using vc4asm via
    // `vc4asm -i vc4.qinc -C ../../gpu_examples/add_vectors.c ../../gpu_examples/add_vectors.qasm`
    unsigned program[] = {
        #include "../../gpu_examples/add_vectors.c"   
    };
    
    // crate 4 vectors to add together
    unsigned int *input1_ptr = malloc(16 * 4);
    unsigned int *input2_ptr = malloc(16 * 4);
    unsigned int *input3_ptr = malloc(16 * 4);
    unsigned int *input4_ptr = malloc(16 * 4);
    for (int i = 0; i < 16; i++) {
        input1_ptr[i] = 2;
        input2_ptr[i] = (unsigned) i;
        input3_ptr[i] = (unsigned) (i + 2);
        input4_ptr[i] = 1;
    }

    // allocate memory for result
    unsigned result_ptr = qpu_malloc(16);

    // lay out uniforms
    unsigned uniforms[] = {(unsigned) input1_ptr, (unsigned) input2_ptr, 
        (unsigned) input3_ptr, (unsigned) input4_ptr, result_ptr};
    
    // run program
    qpu_run(program, SIZE(program), uniforms, 5); 
    timer_delay(1);
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    printf("Result vector: ");
    for (int j=0; j < 16; j++) {
        printf("0x%x ", *((unsigned int*)(result_ptr + 4 * j)));
    }
    printf("\n");

    free(input1_ptr);
    free(input2_ptr);
    free(input3_ptr);
    free(input4_ptr);
    qpu_free(result_ptr);

}

/*
 * Function: populate_life
 * --------------------------
 * Helper function that populates a game of life grid with a start state.
 */
static void populate_life(void *ptr, unsigned int grid_bordered_width, unsigned int on_state)
{
    unsigned int (*in_2d)[grid_bordered_width] = ptr;

    in_2d[2][2] = on_state;
    in_2d[2][3] = on_state;
    in_2d[3][2] = on_state;
    in_2d[3][3] = on_state;

    in_2d[2][5] = on_state;
    in_2d[2][6] = on_state;
    in_2d[2][7] = on_state;

    in_2d[2][9] = on_state;
    in_2d[1][10] = on_state;
    in_2d[3][10] = on_state;
    in_2d[1][11] = on_state;
    in_2d[3][11] = on_state;
    in_2d[2][12] = on_state;

    in_2d[1][14] = on_state;

    in_2d[1][17] = on_state;
    in_2d[2][17] = on_state;
    in_2d[3][17] = on_state;

    in_2d[2][21] = on_state;
    in_2d[2][22] = on_state;
    in_2d[2][23] = on_state;

    in_2d[2][25] = on_state;
    in_2d[2][26] = on_state;
    in_2d[3][25] = on_state;
    in_2d[3][26] = on_state;

    in_2d[1][30] = on_state;
    in_2d[2][31] = on_state;
    in_2d[3][32] = on_state;
}

/*
 * Function: run_toy_life
 * --------------------------
 * Run a game of life in a malloc-ed array of 1s and 0s, accessed as a 2D array,
 * for one iteration.
 * 
 * Input determined by `populate_life()`
 * 0 0 0 0 0 0 0 0 0 1 1 0 0 1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 
 * 0 1 1 0 1 1 1 0 1 0 0 1 0 0 0 0 1 0 0 0 1 1 1 0 1 1 0 0 0 0 1 0 
 * 0 1 1 0 0 0 0 0 0 1 1 0 0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 0 0 0 0 1 
 * 
 * Expected output
 * 0 0 0 0 0 1 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 
 * 0 1 1 1 0 1 0 0 1 0 0 1 0 0 0 1 1 1 0 0 0 1 0 1 1 1 0 0 0 0 1 0 
 * 0 1 1 1 0 1 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 1 0 1 1 1 0 0 0 0 0 0 
 */
void run_toy_life(void)
{
    // load the program, which should be assembled using vc4asm via
    // `vc4asm -i vc4.qinc -C ../../gpu_examples/toy_life.c ../../gpu_examples/toy_life.qasm`
    unsigned program[] = {
        #include "../../gpu_examples/toy_life.c"   
    };

    unsigned int number_of_uniforms = 5;
    
    // 2D array that has 3 rows and 32 columns
    unsigned int grid_width = 32;
    unsigned int grid_height = 3;
    unsigned int grid_bordered_width = grid_width + 2;
    unsigned int grid_bordered_height = grid_height + 2;
    unsigned int *input_ptr = malloc(4 * (grid_bordered_width * grid_bordered_height));
    unsigned int (*in_2d)[grid_bordered_width] = (void *)input_ptr;

    unsigned int *next_ptr = malloc(4 * (grid_bordered_width * grid_bordered_height));
    unsigned int (*next_2d)[grid_bordered_width] = (void *)next_ptr;

    // populate grid
    for (int i = 0; i < grid_bordered_height * grid_bordered_width; i++) {
        input_ptr[i] = 0;
    }
    // make shapes
    populate_life(input_ptr, grid_bordered_width, 1);

    for (int r = 0; r < grid_height; r++) {
        // move through 16 columns at a time
        for (int c = 0; c < grid_width; c++) {
            printf("%d ", in_2d[r + 1][c + 1]);
        }
        printf("\n");
    }
    printf("\n");
    // move through each row

    for (int r = 0; r < grid_height; r++) {
        for (int c = 0; c < grid_width; c+=16) {
            qpu_init();
                
            // printf("r %d, c %d\n", r, c);


            // get pointers to a cell at (1, 1) aka unpadded (0, 0) and its 3 left neighbors
            unsigned int *cell = input_ptr + grid_bordered_width * (r + 1) + (c + 1);
            unsigned int *nw_neigh = input_ptr + grid_bordered_width * (r) + (c);
            unsigned int *w_neigh = input_ptr + grid_bordered_width * (r + 1) + (c);
            unsigned int *sw_neigh = input_ptr + grid_bordered_width * (r + 2) + (c);

            unsigned result_ptr = qpu_malloc(16);
            unsigned uniforms[] = {
                (unsigned) cell, 
                (unsigned) nw_neigh, 
                (unsigned) w_neigh, 
                (unsigned) sw_neigh, 
                result_ptr};
            
            qpu_run(program, SIZE(program), uniforms, number_of_uniforms); 
            
            assert(qpu_request_count() == qpu_complete_count());
            assert(qpu_complete_count() == 1);

            // store results back in the grid
            for (int j=0; j < 16; j++) {
                next_2d[r + 1][c + 1 + j] = *((unsigned int*)(result_ptr + 4*j));
            }
            
            qpu_free(result_ptr);

        }
    }

    for (int r = 0; r < grid_height; r++) {
        // move through 16 columns at a time
        for (int c = 0; c < grid_width; c++) {
            printf("%d ", next_2d[r + 1][c + 1]);
        }
        printf("\n");
    }
    printf("\n");

    free(input_ptr);
    free(next_ptr);

    uart_putchar(EOT);
}

/*
 * Function: write_into_framebuffer
 * --------------------------
 * Tests if the GPU can write into the framebuffer. It passes the GPU the pointer to 
 * the fb and the input pointer of what to write.
 */
void write_into_framebuffer(void)
{
    // initialize the frame buffer
    unsigned int screen_width = 16;
    unsigned int screen_height = 16;
    gl_init(screen_width, screen_height, GL_DOUBLEBUFFER); // initialize frame buffer
    unsigned int padded_width = fb_get_pitch() / fb_get_depth(); 

    unsigned int colors[2] = {GL_BLACK, GL_WHITE};
    gl_clear(colors[0]);
    gl_swap_buffer();
    gl_clear(colors[0]);
    gl_swap_buffer();

    unsigned int *state = fb_get_draw_buffer();
    unsigned int (*state_2d)[padded_width] = (void *) state;

    // write some pixels using the CPU
    state_2d[0][0] = colors[1];
    state_2d[1][1] = colors[1];
    state_2d[2][2] = colors[1];
    state_2d[3][3] = colors[1];
    state_2d[4][2] = colors[1];

    // write 1 row of pixels using the GPU
    // (load a uniform for the FB addr and color, write it to the framebuffer)    
    qpu_init();

    // load the program, which should be assembled using vc4asm via
    // `vc4asm -i vc4.qinc -C ../../gpu_examples/write_fb.c ../../gpu_examples/write_fb.qasm`
    unsigned program[] = {
        #include "../../gpu_examples/write_fb.c"   
    };

    unsigned int *input_ptr = malloc(16 * 4);
    for (int i = 0; i < 16; i++) {
        input_ptr[i] = GL_BLACK;
    }
    input_ptr[0] = GL_YELLOW;
    input_ptr[1] = GL_RED;
    input_ptr[2] = GL_GREEN;
    input_ptr[10] = GL_BLUE;

    unsigned result_ptr = (unsigned) fb_get_draw_buffer();
    unsigned uniforms[] = {(unsigned) input_ptr, result_ptr};
    
    qpu_run(program, SIZE(program), uniforms, SIZE(uniforms)); 
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    printf("Result vector: ");
    for (int j=0; j < 16; j++) {
        printf("0x%x ", *((unsigned int*)(result_ptr + 4 * j)));
    }
    printf("\n");

    qpu_free(result_ptr);
    free(input_ptr);

    gl_swap_buffer();

    uart_putchar(EOT);
}

/*
 * Function: run_fb_life
 * --------------------------
 * Test to run game of life using the frame buffer.
 */
void run_fb_life(void)
{
    unsigned program[] = {
        #include "../qasm/fb_life.c"   
    };
    
    // 2D array that has 3 rows and 32 columns
    unsigned int grid_width = 32;
    unsigned int grid_height = 3;
    unsigned int grid_bordered_width = grid_width + 2;
    unsigned int grid_bordered_height = grid_height + 2;

    // initialize frame buffer (including dead border)
    gl_init(grid_bordered_width, grid_bordered_height, GL_DOUBLEBUFFER); 
    unsigned int fb_padded_width = fb_get_pitch() / fb_get_depth(); 

    // GOL settings
    unsigned int colors[2] = {GL_BLACK, GL_WHITE};
    gl_clear(colors[0]);
    gl_swap_buffer();
    gl_clear(colors[0]);
    gl_swap_buffer();

    // populate grid with shapes
    unsigned int *cur_state = fb_get_draw_buffer();
    populate_life(cur_state, fb_padded_width, colors[1]);

    gl_swap_buffer();
    timer_delay(1);
    unsigned int *next_state = fb_get_draw_buffer();

    unsigned int (*cur_2d)[fb_padded_width] = (void *)cur_state;
    unsigned int (*next_2d)[fb_padded_width] = (void *)next_state;

    // move a sliding window of 16 through the grid
    // coordinates in terms of what is displayed (excluding border)
    // do r + 1 and c + 1 to see true coordinates (including the border)
    unsigned int number_of_uniforms = 7; 
    for (int r = 0; r < grid_height; r++) {
        for (int c = 0; c < grid_width; c+=16) {
            qpu_init();
            
            // get pointers to a cell at (1, 1) aka unpadded (0, 0) and its 3 left neighbors
            unsigned int *cell = cur_state + fb_padded_width * (r + 1) + (c + 1);
            unsigned int *nw_neigh = cur_state + fb_padded_width * (r) + (c);
            unsigned int *w_neigh = cur_state + fb_padded_width * (r + 1) + (c);
            unsigned int *sw_neigh = cur_state + fb_padded_width * (r + 2) + (c);

            // get update address
            unsigned result_ptr = (unsigned) (next_state + fb_padded_width * (r + 1) + (c + 1));
            unsigned uniforms[] = {
                colors[0], colors[1],
                (unsigned) cell, 
                (unsigned) nw_neigh, 
                (unsigned) w_neigh, 
                (unsigned) sw_neigh, 
                result_ptr};
            
            qpu_run(program, SIZE(program), uniforms, number_of_uniforms); 
            
            assert(qpu_request_count() == qpu_complete_count());
            assert(qpu_complete_count() == 1);

        }
    }
    gl_swap_buffer();
    timer_delay(1);
    uart_putchar(EOT);
}

/*
 * Function: populate_vol_life
 * --------------------------
 * Helper function to populate game of life grid, which accepts a volatile pointer.
 */
static void populate_vol_life(volatile unsigned int *ptr, unsigned int grid_bordered_width, unsigned int on_state)
{
    unsigned int (*in_2d)[grid_bordered_width] = (void *) ptr;

    in_2d[2][2] = on_state;
    in_2d[2][3] = on_state;
    in_2d[3][2] = on_state;
    in_2d[3][3] = on_state;

    in_2d[2][5] = on_state;
    in_2d[2][6] = on_state;
    in_2d[2][7] = on_state;

    in_2d[2][9] = on_state;
    in_2d[1][10] = on_state;
    in_2d[3][10] = on_state;
    in_2d[1][11] = on_state;
    in_2d[3][11] = on_state;
    in_2d[2][12] = on_state;

    in_2d[1][14] = on_state;

    in_2d[1][17] = on_state;
    in_2d[2][17] = on_state;
    in_2d[3][17] = on_state;

    in_2d[2][21] = on_state;
    in_2d[2][22] = on_state;
    in_2d[2][23] = on_state;

    in_2d[2][25] = on_state;
    in_2d[2][26] = on_state;
    in_2d[3][25] = on_state;
    in_2d[3][26] = on_state;

    in_2d[1][30] = on_state;
    in_2d[2][31] = on_state;
    in_2d[3][32] = on_state;
}

/*
 * Function: run_fb_life_stepped
 * --------------------------
 * Test to run game of life using the frame buffer for `num_steps` steps.
 */
void run_fb_life_stepped(unsigned int num_steps)
{
    unsigned int number_of_uniforms = 7; 

    unsigned program[] = {
        #include "../qasm/fb_life.c"   
    };
    
    // 2D array that has 3 rows and 32 columns
    unsigned int grid_width = 32;
    unsigned int grid_height = 3;
    unsigned int grid_bordered_width = grid_width + 2;
    unsigned int grid_bordered_height = grid_height + 2;

    // initialize frame buffer (including dead border)
    gl_init(grid_bordered_width, grid_bordered_height, GL_DOUBLEBUFFER); 
    unsigned int fb_padded_width = fb_get_pitch() / fb_get_depth(); 

    // GOL settings
    unsigned int colors[2] = {GL_BLACK, GL_WHITE};
    gl_clear(colors[0]);
    gl_swap_buffer();
    gl_clear(colors[0]);
    gl_swap_buffer();

    // populate grid with shapes
    volatile unsigned int *cur_state = fb_get_draw_buffer();
    populate_vol_life(cur_state, fb_padded_width, colors[1]);

    // show start state
    gl_swap_buffer(); 
    timer_delay(1);
    volatile unsigned int *next_state = fb_get_draw_buffer();

    // determine next state, based on cur state

    // move a sliding window of 16 through the grid
    // coordinates in terms of what is displayed (excluding border)
    // do r + 1 and c + 1 to see true coordinates (including the border)

    for (int i = 0; i < 5; i++) {
        for (int r = 0; r < grid_height; r++) {
            for (int c = 0; c < grid_width; c+=16) {
                qpu_init();
                unsigned uniforms[] = {
                    // get pointers to a cell at (1, 1) aka unpadded (0, 0) and its 3 left neighbors
                    colors[0], colors[1],
                    (unsigned) (cur_state + fb_padded_width * (r + 1) + (c + 1)), 
                    (unsigned) (cur_state + fb_padded_width * (r) + (c)), 
                    (unsigned) (cur_state + fb_padded_width * (r + 1) + (c)), 
                    (unsigned) (cur_state + fb_padded_width * (r + 2) + (c)), 
                    // get update address
                    (unsigned) (next_state + fb_padded_width * (r + 1) + (c + 1)) 
                };
                qpu_run(program, SIZE(program), uniforms, number_of_uniforms); 
                assert(qpu_request_count() == qpu_complete_count());
                assert(qpu_complete_count() == 1);
            }
        }
        gl_swap_buffer(); // display the next state
        timer_delay(1);
        
        cur_state = next_state;
        next_state = fb_get_draw_buffer();
        
    }    

    timer_delay(1);
    uart_putchar(EOT);

}

/*
 * Function: run_fb_life_nodriver
 * --------------------------
 * Test to run game of life using the frame buffer with NO C driver code.
 * (All the logic for looping is moved into the assembly itself.)
 * 
 * It writes some colored pixels in the border region to allow easier reading.
 */
void run_fb_life_nodriver(void)
{
    unsigned program[] = {
        #include "../qasm/life_driver.c"   
    };
    
    // 2D array that has 3 rows and 32 columns
    unsigned int grid_width = 32;
    unsigned int grid_height = 4;
    unsigned int grid_bordered_width = grid_width + 2;
    unsigned int grid_bordered_height = grid_height + 2;

    // initialize frame buffer (including dead border)
    gl_init(grid_bordered_width, grid_bordered_height, GL_DOUBLEBUFFER); 
    unsigned int fb_padded_width = fb_get_pitch() / fb_get_depth(); 

    // GOL settings
    unsigned int colors[2] = {GL_BLACK, GL_WHITE};
    gl_clear(colors[0]);
    gl_swap_buffer();
    gl_clear(colors[0]);
    gl_swap_buffer();

    // populate grid with shapes
    unsigned int *cur_state = fb_get_draw_buffer();
    populate_life(cur_state, fb_padded_width, colors[1]);

    gl_swap_buffer();
    timer_delay(1);
    unsigned int *next_state = fb_get_draw_buffer();

    unsigned int (*cur_2d)[fb_padded_width] = (void *)cur_state;
    unsigned int (*next_2d)[fb_padded_width] = (void *)next_state;

    // move a sliding window of 16 through the grid
    // coordinates in terms of what is displayed (excluding border)
    // do r + 1 and c + 1 to see true coordinates (including the border)
    unsigned int number_of_uniforms = 8; 

    qpu_init();

    unsigned result_ptr = qpu_malloc(16);
    unsigned uniforms[] = {
      (unsigned) grid_height, // number of rows
      (unsigned) grid_width, // number of columns
      (unsigned) fb_padded_width, // fb padded width
      (unsigned) colors[0], // off color
      (unsigned) colors[1], // on color
      (unsigned) cur_state, // prev state
      (unsigned) next_state, // next state
      result_ptr
    };
    qpu_run(program, SIZE(program), uniforms, number_of_uniforms); 
    while (qpu_request_count() != qpu_complete_count()) {}
    assert(qpu_request_count() == qpu_complete_count());
    assert(qpu_complete_count() == 1);    
    
    gl_swap_buffer();
    
    timer_delay(1);

    qpu_free(result_ptr);
    uart_putchar(EOT);
}

void main(void)
{
    uart_init(); 

    // General GPU examples
    // run_helloworld();
    // run_add_vectors();

    // Game of Life examples
    // run_toy_life();
    // write_into_framebuffer();
    // run_fb_life();
    // run_fb_life_stepped(4);
    run_fb_life_nodriver();

    uart_putchar(EOT);
}
