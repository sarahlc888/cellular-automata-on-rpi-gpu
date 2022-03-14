/* Code taken directly from past CS107E project ahconkey-JoshFrancisCodes-project 
 * (originally from their file `project-app.c`)
 *
 * Slightly modified to run without depending on project-module module, and to use an
 * alternate random number module.
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


// function from https://github.com/cs107e/ahconkey-JoshFrancisCodes-project to support pi_approx.c example
void *generate_points(size_t N, size_t w) {
    random_init();
    timer_delay(2);
    unsigned *pts = malloc(2*N);
    for (size_t i = 0; i < 2*N; i++) {
        pts[i] = random_getNumber(0, 200000) % w;
    }
    return pts;
}

// use uniforms for input, initially 
void run_vectorsum(void)
{
    uart_init(); 
    qpu_init();

    //this is how we include programs
    // the assembled program is taken directly from https://rpiplayground.wordpress.com/2014/05/03/hacking-the-gpu-for-fun-and-profit-pt-1/
    unsigned program[] = {
        #include "vectorsum.c"   
    };
    
    unsigned result_ptr = qpu_malloc(16);
    unsigned uniforms[] = {result_ptr, 101};
    for (int j=0; j < 68; j++) {
        *((unsigned int*)(result_ptr + j)) = 0;
        // printf("--word %d: %d\n", j, *((unsigned int*)(result_ptr + j)));
    }
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    qpu_run(program, SIZE(program), uniforms, 2);
    
    timer_delay(1);

    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    printf("Result: %d\n", *(volatile unsigned *) result_ptr );
    for (int j=0; j < 16; j++) {
        printf("word %d: %d\n", j, *((unsigned int*)(result_ptr + j)));
    }
    for (int j=0; j < 68; j+=4) {
        printf("--word %d: %d\n", j, *((unsigned int*)(result_ptr + j)));
    }

    qpu_free(result_ptr);

    uart_putchar(EOT);
}


// use uniforms for input, initially 
void run_deadbeef(void)
{
    uart_init(); 
    qpu_init();

    //this is how we include programs
    // the assembled program is taken directly from https://rpiplayground.wordpress.com/2014/05/03/hacking-the-gpu-for-fun-and-profit-pt-1/
    unsigned program[] = {
        #include "deadbeef.c"   
    };
    
    unsigned result_ptr = qpu_malloc(16);
    unsigned uniforms[] = {result_ptr};
    for (int j=0; j < 68; j++) {
        *((unsigned int*)(result_ptr + j)) = 0;
        // printf("--word %d: %d\n", j, *((unsigned int*)(result_ptr + j)));
    }
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    qpu_run(program, SIZE(program), uniforms, 1);
    
    timer_delay(1);

    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    printf("Result: %d\n", *(volatile unsigned *) result_ptr );
    for (int j=0; j < 16; j++) {
        printf("word %d: %d\n", j, *((unsigned int*)(result_ptr + j)));
    }
    for (int j=0; j < 68; j+=4) {
        printf("--word %d: %d\n", j, *((unsigned int*)(result_ptr + j)));
    }

    qpu_free(result_ptr);

    uart_putchar(EOT);
}

// run helloworld program from https://rpiplayground.wordpress.com/2014/05/03/hacking-the-gpu-for-fun-and-profit-pt-1/
void run_helloworld(void)
{
    uart_init(); 
    qpu_init();

    //this is how we include programs
    // the assembled program is taken directly from https://rpiplayground.wordpress.com/2014/05/03/hacking-the-gpu-for-fun-and-profit-pt-1/
    unsigned program[] = {
        #include "helloworld.c"   
    };
    
    unsigned result_ptr = qpu_malloc(16);
    unsigned uniforms[] = {101, result_ptr};
    
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    qpu_run(program, SIZE(program), uniforms, 2);
    
    timer_delay(1);

    printf("Requested: %d\n", qpu_request_count());
    printf("Commpleted: %d\n", qpu_complete_count());

    printf("Result: %d\n", *(volatile unsigned *) result_ptr );

    qpu_free(result_ptr);


    qpu_print_status();
    // qpu_init();
    // qpu_print_status();
    unsigned result_ptr2 = qpu_malloc(16);
    unsigned uniforms2[] = {201, result_ptr2};
    
    unsigned * comb_uniforms2 = (unsigned *) malloc(2 * 4);
    memcpy(comb_uniforms2, uniforms2, 2);

    printf("Requested: %d\n", qpu_request_count());
    printf("Commpleted: %d\n", qpu_complete_count());

    qpu_run(program, SIZE(program), uniforms2, 2);
    
    timer_delay(1);

    printf("Requested: %d\n", qpu_request_count());
    printf("Commpleted: %d\n", qpu_complete_count());

    printf("Result: %d\n", *(volatile unsigned *) result_ptr2 );


    qpu_free(result_ptr2);
    free(comb_uniforms2);
    uart_putchar(EOT);
}

void run_basic(void)
{
    uart_init(); 
    qpu_init();
    qpu_print_status();
    unsigned program[] = {
        #include "basic.c"   
    };
    
    unsigned int *input1_ptr = malloc(16 * 4);
    unsigned int *input2_ptr = malloc(16 * 4);
    for (int i = 0; i < 16; i++) {
        input1_ptr[i] = 18;
        input2_ptr[i] = 12;
    }
    unsigned result_ptr = qpu_malloc(16);
    unsigned uniforms[] = {(unsigned) input1_ptr, (unsigned) input2_ptr, result_ptr};
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    qpu_run(program, SIZE(program), uniforms, 3);
    
    timer_delay(1);

    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    // printf("Result: %d\n", *(volatile unsigned *) result_ptr );
    for (int j=0; j <= 16; j++) {
        printf("word %d: %d\n", j, *((unsigned int*)(result_ptr + 4*j)));
    }
    free(input1_ptr);
    free(input2_ptr);
    qpu_free(result_ptr);

    uart_putchar(EOT);
}

void run_basic_input(void)
{
    uart_init(); 
    qpu_init();

    unsigned program[] = {
        #include "basic_input.c"   
    };
    
    unsigned int *input1_ptr = malloc(16 * 4);
    unsigned int *input2_ptr = malloc(16 * 4);
    unsigned int *input3_ptr = malloc(16 * 4);
    unsigned int *input4_ptr = malloc(16 * 4);
    // FLAG change
    for (int i = 0; i < 16; i++) {
        input1_ptr[i] = 2;
        input2_ptr[i] = (unsigned) i;
        input3_ptr[i] = (unsigned) (i + 2);
        input4_ptr[i] = 1;
    }
    unsigned result_ptr = qpu_malloc(16);
    unsigned uniforms[] = {(unsigned) input1_ptr, (unsigned) input2_ptr, 
        (unsigned) input3_ptr, (unsigned) input4_ptr, result_ptr};
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    timer_delay(1);
    qpu_print_status();
    qpu_run(program, SIZE(program), uniforms, 5); // FLAG change
    
    timer_delay(1);

    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    // printf("Result: %d\n", *(volatile unsigned *) result_ptr );
    // for (int j=0; j <= 16; j++) {
    //     printf("word %d: %d\n", j, *((unsigned int*)(result_ptr + j)));
    // }
    // printf("\n");
    for (int j=0; j <= 16; j++) {
        printf("word %d: %d\n", j, *((unsigned int*)(result_ptr + 4*j)));
    }
    free(input1_ptr);
    free(input2_ptr);
    free(input3_ptr);
    free(input4_ptr);
    // FLAG change
    qpu_free(result_ptr);

    uart_putchar(EOT);
}

void run_2d_input(void)
{
    uart_init(); 
    qpu_init();

    unsigned program[] = {
        #include "2d_input.c"   
    };

    unsigned int number_of_uniforms = 5;
    
    // 2D array that has 3 rows and 16 columns
    unsigned int grid_rows = 3;
    unsigned int grid_cols = 16;
    unsigned int *input_ptr = malloc(4 * (grid_rows * grid_cols + 2));

    // get pointers to a cell at (1, 1) and its 3 left neighbors
    unsigned int *cell = input_ptr + grid_cols + 1;
    unsigned int *nw_neigh = input_ptr;
    unsigned int *w_neigh = input_ptr + grid_cols;
    unsigned int *sw_neigh = input_ptr + 2 * grid_cols;

    // populate grid
    for (int i = 0; i < grid_rows * grid_cols + 2; i++) {
        input_ptr[i] = 0;
        // input_ptr[i] = i;
    }
    // make shapes
    input_ptr[17] = 1;
    input_ptr[18] = 1;
    input_ptr[33] = 1;
    input_ptr[34] = 1;

    input_ptr[20] = 1;
    input_ptr[21] = 1;
    input_ptr[22] = 1;

    input_ptr[24] = 1;
    input_ptr[9] = 1;
    input_ptr[10] = 1;
    input_ptr[27] = 1;
    input_ptr[41] = 1;
    input_ptr[42] = 1;

    input_ptr[14] = 1;
    input_ptr[31] = 1;
    input_ptr[47] = 1;

    printf("nw: %d, w: %d, sw:%d\n", *nw_neigh, *w_neigh, *sw_neigh);

    unsigned result_ptr = qpu_malloc(16);
    unsigned uniforms[] = {
        (unsigned) cell, 
        (unsigned) nw_neigh, 
        (unsigned) w_neigh, 
        (unsigned) sw_neigh, 
        result_ptr};
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    timer_delay(1);
    qpu_print_status();
    qpu_run(program, SIZE(program), uniforms, number_of_uniforms); // FLAG change
    
    timer_delay(1);

    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    // printf("Result: %d\n", *(volatile unsigned *) result_ptr );
    // for (int j=0; j <= 16; j++) {
    //     printf("word %d: %d\n", j, *((unsigned int*)(result_ptr + j)));
    // }
    // printf("\n");
    for (int j=0; j <= 16; j++) {
        printf("word %d: %d\n", j, *((unsigned int*)(result_ptr + 4*j)));
    }
    free(input_ptr);
    // FLAG change
    qpu_free(result_ptr);

    uart_putchar(EOT);
}

// Test game of life in malloc-ed memory of 1s and 0s
void populate_life(void *ptr, unsigned int grid_padded_width)
{
    unsigned int (*in_2d)[grid_padded_width] = ptr;

    in_2d[2][2] = 1;
    in_2d[2][3] = 1;
    in_2d[3][2] = 1;
    in_2d[3][3] = 1;

    in_2d[2][5] = 1;
    in_2d[2][6] = 1;
    in_2d[2][7] = 1;

    in_2d[2][9] = 1;
    in_2d[1][10] = 1;
    in_2d[3][10] = 1;
    in_2d[1][11] = 1;
    in_2d[3][11] = 1;
    in_2d[2][12] = 1;

    in_2d[1][14] = 1;

    in_2d[1][17] = 1;
    in_2d[2][17] = 1;
    in_2d[3][17] = 1;

    in_2d[2][21] = 1;
    in_2d[2][22] = 1;
    in_2d[2][23] = 1;

    in_2d[2][25] = 1;
    in_2d[2][26] = 1;
    in_2d[3][25] = 1;
    in_2d[3][26] = 1;

    in_2d[1][30] = 1;
    in_2d[2][31] = 1;
    in_2d[3][32] = 1;
}
void run_toy_life(void)
{
    // Expected input
    // 0 0 0 0 0 0 0 0 0 1 1 0 0 1 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 
    // 0 1 1 0 1 1 1 0 1 0 0 1 0 0 0 0 1 0 0 0 1 1 1 0 1 1 0 0 0 0 1 0 
    // 0 1 1 0 0 0 0 0 0 1 1 0 0 0 0 0 1 0 0 0 0 0 0 0 1 1 0 0 0 0 0 1 

    // Expected output
    // 0 0 0 0 0 1 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 
    // 0 1 1 1 0 1 0 0 1 0 0 1 0 0 0 1 1 1 0 0 0 1 0 1 1 1 0 0 0 0 1 0 
    // 0 1 1 1 0 1 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 1 0 1 1 1 0 0 0 0 0 0 


    uart_init(); 

    unsigned program[] = {
        #include "toy_life.c"   
    };

    unsigned int number_of_uniforms = 5;
    
    // 2D array that has 3 rows and 32 columns
    unsigned int grid_width = 32;
    unsigned int grid_height = 3;
    unsigned int grid_padded_width = grid_width + 2;
    unsigned int grid_padded_height = grid_height + 2;
    unsigned int *input_ptr = malloc(4 * (grid_padded_width * grid_padded_height));
    unsigned int (*in_2d)[grid_padded_width] = (void *)input_ptr;

    unsigned int *next_ptr = malloc(4 * (grid_padded_width * grid_padded_height));
    unsigned int (*next_2d)[grid_padded_width] = (void *)next_ptr;

    // populate grid
    for (int i = 0; i < grid_padded_height * grid_padded_width; i++) {
        input_ptr[i] = 0;
    }
    // make shapes
    populate_life(input_ptr, grid_padded_width);

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
            unsigned int *cell = input_ptr + grid_padded_width * (r + 1) + (c + 1);
            unsigned int *nw_neigh = input_ptr + grid_padded_width * (r) + (c);
            unsigned int *w_neigh = input_ptr + grid_padded_width * (r + 1) + (c);
            unsigned int *sw_neigh = input_ptr + grid_padded_width * (r + 2) + (c);

            // printf("%d\n", *cell);

            unsigned result_ptr = qpu_malloc(16);
            unsigned uniforms[] = {
                (unsigned) cell, 
                (unsigned) nw_neigh, 
                (unsigned) w_neigh, 
                (unsigned) sw_neigh, 
                result_ptr};
            
            qpu_run(program, SIZE(program), uniforms, number_of_uniforms); 
            
            assert(qpu_request_count() == qpu_complete_count());
            // completed++;
            assert(qpu_complete_count() == 1);

            // TODO: store results back in the grid
            for (int j=0; j < 16; j++) {
                // printf("%d ", *((unsigned int*)(result_ptr + 4*j)));
                next_2d[r + 1][c + 1 + j] = *((unsigned int*)(result_ptr + 4*j));
            }
            // printf("--\n");
            
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

// Test if the GPU can write into the framebuffer
void write_into_framebuffer(void)
{
    uart_init(); 

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

    // write 1 pixel using the CPU
    state_2d[0][0] = colors[1];
    state_2d[1][1] = colors[1];
    state_2d[2][2] = colors[1];
    state_2d[3][3] = colors[1];
    state_2d[4][2] = colors[1];



    // write 1 row of pixels using the GPU
    // (load a uniform for the FB addr and color, write it to the framebuffer)
    int num_uniforms = 2;
    
    qpu_init();
    unsigned program[] = {
        #include "write_fb.c"   
    };
    
    unsigned int *input_ptr = malloc(16 * 4);
    for (int i = 0; i < 16; i++) {
        input_ptr[i] = GL_BLACK;
    }
    input_ptr[0] = GL_YELLOW;
    input_ptr[1] = GL_RED;
    input_ptr[2] = GL_GREEN;
    input_ptr[10] = GL_BLUE;

    // unsigned result_ptr = qpu_malloc(16); //fb_get_draw_buffer();
    unsigned result_ptr = (unsigned) fb_get_draw_buffer();
    unsigned uniforms[] = {(unsigned) input_ptr, result_ptr};
    
    qpu_run(program, SIZE(program), uniforms, num_uniforms); // FLAG change
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    for (int j=0; j <= 16; j++) {
        printf("word %d: %08x\n", j, *((unsigned int*)(result_ptr + 4*j)));
    }
    qpu_free(result_ptr);
    free(input_ptr);

    gl_swap_buffer();

    uart_putchar(EOT);
}

void main(void)
{
    // run_helloworld();
    // run_vectorsum();
    // run_deadbeef();
    // run_basic();
    // run_basic();
    // run_basic_input();
    // run_basic_input();

    // run_2d_input();
    // run_2d_input();

    // run_toy_life();

    write_into_framebuffer();
}
