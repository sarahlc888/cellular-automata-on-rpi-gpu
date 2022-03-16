#include "../../include/button.h"
#include "../../include/ca_gpu.h"
#include "../../include/draw_ca.h"
#include "../../include/profile.h"
#include "../../include/read_write_ca.h"
#include "../../include/system.h"
#include "gl.h"
#include "gpio_extra.h"
#include "interrupts.h"
#include "printf.h"
#include "timer.h"
#include "uart.h"
#include "ff.h"

// TODO: make this depend only on the top level library. Use typedef enum to
// specify which preset function to use, rather than the function pointer itself

// adapted from $CS107E/examples/sd_fatfs
// TODO: clear this up or maybe remove. Def need to test it in the context of ca_gpu or whatever
void run_fs_tests(void) 
{
    uart_init();
    printf("Starting libpisd.a test\n");

    printf("initializing\n");

    FATFS fs;
    ca_ffs_init(&fs);

    printf("scanning\n");
    int n = recursive_scan(""); // start at root
    printf("Scan found %d entries.\n\n", n);

    make_dir("/presets");

    color_t writebuf[] = {GL_BLACK, GL_WHITE, GL_RED, GL_ORANGE};
    unsigned int bytes = sizeof writebuf;
    color_t readbuf[bytes];
    write_preset(writebuf, bytes, "/presets/curiously_long_filename.rgba");
    read_preset(readbuf, bytes, "/presets/curiously_long_filename.rgba");
    n = recursive_scan("/"); // start at root
    remove_preset("/presets/curiously_long_filename.rgba");
    n = recursive_scan("/"); // start at root
}


void main(void) {
  system_enable_cache();

  unsigned int delay_ms = 0;
  unsigned int run_time = 1000000 * 10;
  unsigned int save_preset = 0;
  unsigned int use_time_limit = 1;
  uart_init();
  timer_init();
  printf("Running tests from file %s\n", __FILE__);

  interrupts_init();
  profile_init();
  button_init(21);
  interrupts_global_enable();

  gpio_set_pullup(21);
  gpio_set_input(21);

  const char *preset_file = "/presets/life_300x300_1.rgba";

  // game of life
  color_t color_states[2] = {GL_BLACK, GL_WHITE};
  for (int i = 1; i < 9; i++) {
    // for (int j = 0; j < 3; j++) {
    printf("Size: %dx%d:\n", 128 * i, 128 * i);
    ca_init(LIFE, 128 * i, 128 * i, color_states,
            delay_ms); // ca_init(LIFE, 1000, 1000, color_states, 1000)

    ca_create_and_load_preset(
        preset_file, (preset_fn_t)create_random_life_preset,
        save_preset); // create_random_life_preset, create_life_preset2
    // ca_create_and_load_preset(preset_file, (preset_fn_t)
    // create_random_life_preset, save_preset);
    // profile_on();
    ca_run(use_time_limit, run_time, 21, 1);

    // wire world
    // color_t color_states[4] = {GL_WHITE, GL_YELLOW, GL_RED, GL_BLACK};
    // ca_init(WIREWORLD, 100, 100, color_states, 100);
    // ca_create_and_load_preset(preset_file, (preset_fn_t) create_ww_preset,
    // 0); ca_run(use_time_limit, run_time);

    if (save_preset) {
      remove_preset(preset_file);
    }
  }

  profile_off();

  uart_putchar(EOT);
}
