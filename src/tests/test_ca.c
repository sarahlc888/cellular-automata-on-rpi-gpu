#include "printf.h"
#include "uart.h"
#include "gl.h"
#include "timer.h"
#include "../../include/ca.h"

void main(void)
{
    uart_init();
    timer_init();
    printf("Running tests from file %s\n", __FILE__);

    color_t color_states[2] = {GL_BLACK, GL_WHITE};
    ca_init(0, 500, 500, 2, color_states, 100);
    // ca_init(0, 500, 500, 2, color_states, 1000);
    ca_run();

    uart_putchar(EOT);
}
