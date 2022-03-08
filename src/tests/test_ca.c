#include "printf.h"
#include "uart.h"
#include "gl.h"
#include "timer.h"
#include "../../include/ca.h"
#include "../../include/draw_ca.h"

// TODO: make this depend only on the top level library. Use typedef enum to specify
// which preset function to use, rather than the function pointer itself

void main(void)
{
    uart_init();
    timer_init();
    printf("Running tests from file %s\n", __FILE__);


    // game of life
    // color_t color_states[2] = {GL_BLACK, GL_WHITE};
    // ca_init(LIFE, 32, 32, color_states, 1000); // ca_init(LIFE, 1000, 1000, color_states, 1000)
    // ca_create_and_load_preset("/presets/life_1.rgba", (preset_fn_t) create_life_preset);
    // ca_run();

    // wire world
    color_t color_states[4] = {GL_WHITE, GL_YELLOW, GL_RED, GL_BLACK};
    ca_init(WIREWORLD, 32, 32, color_states, 100);
    ca_create_and_load_preset("/presets/ww_1.rgba", (preset_fn_t) create_ww_preset);
    ca_run();

    uart_putchar(EOT);
}
