/* Code taken directly from past CS107E project ahconkey-JoshFrancisCodes-project 
 * (originally from their file `project-app.c`)
 *
 * Slightly modified to run without depending on project-module module, and to use an
 * alternate random number module.
 */

#include "printf.h"
#include "uart.h"
#include "malloc.h"
#include "interrupts.h"
#include "timer.h"
#include "strings.h"
#include "../../include/mailbox_functions.h"
#include "../../include/qpu.h"
#include "../../include/randomHardware.h"

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

    unsigned program[] = {
        #include "basic.c"   
    };
    
    unsigned result_ptr = qpu_malloc(16);
    unsigned uniforms[] = {result_ptr};
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    qpu_run(program, SIZE(program), uniforms, 1);
    
    timer_delay(1);

    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    // printf("Result: %d\n", *(volatile unsigned *) result_ptr );
    for (int j=0; j <= 16; j++) {
        printf("word %d: %d\n", j, *((unsigned int*)(result_ptr + 4*j)));
    }
    qpu_free(result_ptr);

    uart_putchar(EOT);
}

void main(void)
{
    // run_helloworld();
    // run_vectorsum();
    // run_deadbeef();
    run_basic();
}
