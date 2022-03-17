/* Sarah Chen
 * 03/15/2022
 * Code for CS107E Final Project
 *
 * This program tests the `read_write_ca` and `ca_gpu` modules. It tests the
 * core functionality of the project. Correctness should be verified through
 * visual inspection. See function comments for details on what to expect.
 */

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

#define MAIN_BUTTON 21

/*
 * Function: test_write_read_preset
 * --------------------------
 * Test the `read_write_ca` module by creating and deleting a file at
 * "/presets/curiously_long_filename.rgba" and viewing the SD card's contents
 * throughout.
 * 
 * Affirm that the results are correct by visual inspection. (The written and
 * read values should match exactly, and the file should be successfully removed.)
 * 
 * The code is adapted from $CS107E/examples/sd_fatfs.
 */
void test_write_read_preset(void) 
{
  printf("Starting FAT FS test to write and read presets\n\nScanning:");

  FATFS fs;
  ca_ffs_init(&fs);

  int n = recursive_scan(""); // start at root
  printf("Scan found %d entries.\n\n", n);

  color_t writebuf[] = {GL_BLACK, GL_WHITE, GL_RED, GL_ORANGE};
  unsigned int bytes = sizeof writebuf;
  color_t readbuf[bytes];
  write_preset(writebuf, bytes, "/presets/curiously_long_filename.rgba");
  print_color_buf(writebuf, bytes / 4);
  read_preset(readbuf, bytes, "/presets/curiously_long_filename.rgba");
  print_color_buf(readbuf, bytes / 4);

  printf("\nScanning:\n");
  n = recursive_scan(""); 
  remove_preset("/presets/curiously_long_filename.rgba");
  printf("\nScanning after removing preset:\n");
  n = recursive_scan("");
}

/*
 * Function: test_large_ww
 * --------------------------
 * Test a large grid with WireWorld, which runs on CPU. There should be a very
 * small OR/XOR/AND gate configuration in the top left corner of the screen,
 * which should run correctly. 
 */
void test_large_ww(unsigned int screen_width)
{
  printf("\n\nTesting WireWorld with large grid size:\n");
  system_enable_cache();

  unsigned int delay_ms = 0;
  unsigned int run_time = 1000000 * 8; // run for 8 seconds
  
  unsigned int use_time_limit = 1;
  unsigned int verbosity = 1;
  unsigned int save_preset = 0; 

  const char *preset_file = "/presets/ww_random_test.rgba";

  color_t color_states[4] = {GL_WHITE, GL_YELLOW, GL_RED, GL_BLACK};

  ca_init(WIREWORLD, screen_width, screen_width, color_states, delay_ms);
  ca_create_and_load_preset(
    preset_file, 
    (preset_fn_t) create_ww_preset,
    0); 
  profile_on();
  ca_run(use_time_limit, run_time, MAIN_BUTTON, verbosity);
  profile_off();

  ca_create_and_load_preset(
      preset_file, 
      (preset_fn_t)create_random_life_preset,
      save_preset); 

  if (save_preset) {
    remove_preset(preset_file);
  }

}

/*
 * Function: test_ca
 * --------------------------
 * Test the `ca_gpu` module by creating random presets for various screen sizes.
 * The module should output the number of updates.
 * 
 * Run each simulation for `run_seconds` seconds.
 * 
 * It does not save the generated presets to the SD card in order to conserve testing
 * time.
 */
void test_ca(unsigned int run_seconds)
{
  printf("\n\nTesting random Game of Life with various grid sizes:\n");
  system_enable_cache();

  unsigned int delay_ms = 0;
  unsigned int run_time = 1000000 * run_seconds;
  
  unsigned int use_time_limit = 1;
  unsigned int verbosity = 1;
  unsigned int save_preset = 0; 
  unsigned int use_profiler = 0;

  const char *preset_file = "/presets/life_random_test.rgba";

  // game of life
  color_t color_states[2] = {GL_BLACK, GL_WHITE};
  for (int i = 1; i < 9; i++) {
    printf("Size: %dx%d:\n", 128 * i, 128 * i);

    ca_init(LIFE, 128 * i, 128 * i, color_states,
            delay_ms); 

    ca_create_and_load_preset(
        preset_file, 
        (preset_fn_t)create_random_life_preset,
        save_preset); 

    if (use_profiler) profile_on();
    ca_run(use_time_limit, run_time, MAIN_BUTTON, verbosity);
    if (use_profiler) profile_off();

    if (save_preset) {
      remove_preset(preset_file);
    }
  }
}

void main(void) {
  printf("Running tests from file %s\n", __FILE__);

  uart_init();

  test_write_read_preset();

  // initialize dependencies
  timer_init();
  interrupts_init();
  profile_init();
  // initialize the button
  button_init(MAIN_BUTTON);
  interrupts_global_enable();
  gpio_set_pullup(MAIN_BUTTON);
  gpio_set_input(MAIN_BUTTON);

  test_large_ww(1264);
  test_ca(7);

  uart_putchar(EOT);
}
