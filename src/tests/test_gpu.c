/* Code taken directly from past CS107E project ahconkey-JoshFrancisCodes-project 
 * (originally from their file `project-app.c`)
 *
 * Slightly modified to run without depending on project-module module, and to use an
 * alternate random number module.
 */

#include "printf.h"
#include "uart.h"
#include "mailbox.h"
#include "malloc.h"
#include "interrupts.h"
#include "timer.h"
#include "strings.h"
#include "../../include/mailbox_functions.h"
#include "../../include/qpu.h"
#include "../../include/randomHardware.h"

#define PHYS_TO_BUS(x) ((x)|0xC0000000)
#define SIZE(x) sizeof(x)/sizeof(x[0])



void *generate_points(size_t N, size_t w) {
    random_init();
    timer_delay(2);
    unsigned *pts = malloc(2*N);
    for (size_t i = 0; i < 2*N; i++) {
        pts[i] = random_getNumber(0, 200000) % w;
    }
    return pts;
}

void main(void)
{
    uart_init(); 
    qpu_init();

    //this is how we include programs
    unsigned program[] = {
        #include "helloworld.c"   
    };
    
    unsigned result_ptr = qpu_malloc(1);
    unsigned uniforms[] = {112, result_ptr};
    
    unsigned * comb_uniforms= (unsigned *) malloc(2 * 4);
    memcpy(comb_uniforms, uniforms, 2);
    
    printf("Requested: %d\n", qpu_request_count());
    printf("Completed: %d\n", qpu_complete_count());

    qpu_run(program, SIZE(program), uniforms, 2);
    
    timer_delay(1);

    printf("Requested: %d\n", qpu_request_count());
    printf("Commpleted: %d\n", qpu_complete_count());

    printf("Result: %d\n", *(volatile unsigned *) result_ptr );
    uart_putchar(EOT);
}

// void main(void)
// {
//     uart_init(); 
//     qpu_init();

//     unsigned program[] = {
//         #include "helloworld.c"   
//     };
    
//     unsigned result_ptr = qpu_malloc(1);
//     printf("at %x value %d\n", result_ptr, *((unsigned *)result_ptr));
//     unsigned uniforms[] = {111, result_ptr};
    
//     unsigned * comb_uniforms= (unsigned *) malloc(2 * 4);
//     memcpy(comb_uniforms, uniforms, 8); // 8 bytes, 2 ints
//     printf("[0]: %d\n", comb_uniforms[0]);
//     printf("[1]: %p\n", (void *)comb_uniforms[1]);
//     printf("[1]: %x\n", comb_uniforms[1]);

//     printf("Requested: %d\n", qpu_request_count());
//     printf("Completed: %d\n", qpu_complete_count());

//     qpu_run(program, SIZE(program), uniforms, 2);
    
//     timer_delay(2);

//     printf("Requested: %d\n", qpu_request_count());
//     // while (qpu_complete_count() == 0) {}
//     printf("Completed: %d\n", qpu_complete_count());

//     printf("Result: %d\n", *(volatile unsigned *) result_ptr );

//     qpu_free(result_ptr);
//     free(comb_uniforms);
//     uart_putchar(EOT);
// }
