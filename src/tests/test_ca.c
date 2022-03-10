#include "printf.h"
#include "uart.h"
#include "gl.h"
#include "timer.h"
#include "../../include/system.h"
#include "../../include/ca.h"
#include "../../include/draw_ca.h"
#include "../../include/read_write_ca.h"

// TODO: make this depend only on the top level library. Use typedef enum to specify
// which preset function to use, rather than the function pointer itself

void main(void)
{
    system_enable_cache();


    unsigned int delay_ms = 0;
    unsigned int run_time = 1000000 * 5;
    unsigned int save_preset = 0;
    unsigned int use_time_limit = 1;
    uart_init();
    timer_init();
    printf("Running tests from file %s\n", __FILE__);

    const char *preset_file = "/presets/life_300x300_1.rgba";

    // game of life
    color_t color_states[2] = {GL_BLACK, GL_WHITE};
    ca_init(LIFE, 100, 100, color_states, delay_ms); // ca_init(LIFE, 1000, 1000, color_states, 1000)

    ca_create_and_load_preset(preset_file, (preset_fn_t) create_life_preset2, save_preset);
    // ca_create_and_load_preset(preset_file, (preset_fn_t) create_random_life_preset, save_preset);
    ca_run(use_time_limit, run_time);

    // wire world
    // color_t color_states[4] = {GL_WHITE, GL_YELLOW, GL_RED, GL_BLACK};
    // ca_init(WIREWORLD, 100, 100, color_states, 100);
    // ca_create_and_load_preset(preset_file, (preset_fn_t) create_ww_preset, 0);
    // ca_run(use_time_limit, run_time);
    
    if (save_preset) {
        remove_preset(preset_file);
    }
    uart_putchar(EOT);
}
