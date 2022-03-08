#include "printf.h"
#include "uart.h"
#include "gl.h"
#include "timer.h"
#include "../../include/ca.h"
#include "../../include/draw_ca.h"
#include "../../include/read_write_ca.h"

// TODO: make this depend only on the top level library. Use typedef enum to specify
// which preset function to use, rather than the function pointer itself

void main(void)
{
    unsigned int delay_ms = 1000;
    unsigned int run_time = 1000000 * 10;
    uart_init();
    timer_init();
    printf("Running tests from file %s\n", __FILE__);

    const char *preset_file = "/presets/life_50x50_1.rgba";

    // game of life
    color_t color_states[2] = {GL_BLACK, GL_WHITE};
    ca_init(LIFE, 50, 50, color_states, delay_ms); // ca_init(LIFE, 1000, 1000, color_states, 1000)

    ca_create_and_load_preset(preset_file, (preset_fn_t) create_life_preset2);
    ca_run(run_time);

    // wire world
    // color_t color_states[4] = {GL_WHITE, GL_YELLOW, GL_RED, GL_BLACK};
    // ca_init(WIREWORLD, 100, 100, color_states, 100);
    // ca_create_and_load_preset(preset_file, (preset_fn_t) create_ww_preset);
    // ca_run(run_time);

    remove_preset(preset_file);
    uart_putchar(EOT);
}
