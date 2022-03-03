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

    ca_init(0, 64, 64, GL_WHITE, GL_BLACK, 1000);
    ca_run();

    uart_putchar(EOT);
}
