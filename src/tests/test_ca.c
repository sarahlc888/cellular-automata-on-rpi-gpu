#include "printf.h"
#include "uart.h"
#include "gl.h"
#include "timer.h"
#include "../../include/ca.h"
#include "../../include/read_write_ca.h"

void main(void)
{
    uart_init();
    timer_init();
    printf("Running tests from file %s\n", __FILE__);


    // game of life
    color_t color_states[2] = {GL_BLACK, GL_WHITE};
    ca_init(LIFE, 16, 16, 2, color_states, 1000);

    // // ca_init(LIFE, 1000, 1000, 2, color_states, 1000);

    // // wire world

    // color_t color_states[4] = {GL_WHITE, GL_YELLOW, GL_RED, GL_BLACK};
    // ca_init(WIREWORLD, 500, 500, 2, color_states, 100);

    // // ca_init(WIREWORLD, 50, 50, 2, color_states, 1000);

    ca_run();



    // color_t test_data[4] = {GL_WHITE, GL_BLACK, GL_ORANGE, GL_GREEN};
    // const char *fname = "test.rbga";
    // color_t test_read[4] = {0};

    // write_state_file(test_data, 16, fname);
    // read_state_file(test_read, 16, fname, 16); // expect 16 bytes

    // run_tests();

    uart_putchar(EOT);
}
