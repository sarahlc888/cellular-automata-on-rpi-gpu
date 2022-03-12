/* Code taken directly from past CS107E project ahconkey-JoshFrancisCodes-project */

#include "strings.h"
#include "../../include/qpu.h"
#include "../../include/mailbox_functions.h"
#include "printf.h"

#define CLOCK_RATE 250*1000*1000

int qpu_init(void) {
    unsigned ret = qpu_enable(0, 1);
    set_clock_rate(CLOCK_RATE);
    qpu_reset_status(); // changed from reset_status()
    
    return ret;
}

unsigned qpu_malloc(unsigned size) {
    unsigned handle = mem_alloc(0, size*sizeof(unsigned), 4096, 0x4);
    return mem_lock(0, handle);
}

unsigned qpu_free(unsigned handle) {
    mem_unlock(0, handle);
    return mem_free(0, handle);
}

int qpu_run(unsigned *code, unsigned code_length, unsigned *uniforms, unsigned uniforms_count) {
    printf("  %d length\n", code_length);
    printf("  %d uniforms_count\n", uniforms_count);
    unsigned code_ptr = qpu_malloc(code_length + uniforms_count);
    printf("  %p code_ptr\n", (void *)code_ptr);

    memcpy((unsigned *)code_ptr, code, code_length * sizeof(unsigned));

    unsigned uniform_ptr = code_ptr + code_length * sizeof(unsigned);
    printf("  %p uniform_ptr\n", (void *)uniform_ptr);
    memcpy((unsigned *)uniform_ptr, uniforms, uniforms_count * sizeof(unsigned));

    // These lines send the program by writing to GPU registers (see qpu.h)
    *((unsigned *) UNIFORMS) = uniform_ptr;
    *((unsigned *) ULENGTH) = uniforms_count;
    *((unsigned *) PROGRAM) = code_ptr;

    qpu_free(code_ptr);

    return 0;
}

unsigned char qpu_request_count() {
    return ((*((volatile unsigned *) V3D_SRQCS)) >> 8) & 0xff;
}

unsigned char qpu_complete_count() {
    return ((*((volatile unsigned *) V3D_SRQCS)) >> 16) & 0xff;
}

void qpu_print_status() {
    printf("%08x\n",*((volatile unsigned *) V3D_SRQCS)); 
}

//further code inspired/drawn from
//https://github.com/raspberrypi/firmware/blob/master/opt/vc/src/hello_pi/hello_fft/gpu_fft_base.c
void qpu_reset_status() {
    *((volatile unsigned *) V3D_SRQCS) = (1<<7) | (1<<8) | (1<<16);
}

void qpu_enable_interrupts() {
    *((volatile unsigned *) V3D_DBQITE) = 0xffff;
}
