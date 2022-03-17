/* Sarah Chen
 * 03/15/2022
 * Code for CS107E Final Project
 *
 * This module supports reading and writing to the Raspberry Pi's SD card.
 */

#ifndef RWCA_H
#define RWCA_H

#include "uart.h"
#include "ff.h"

/*
 * Function: ca_ffs_init
 * --------------------------
 * Initialize the file system by mounting the SD card.
 */
void ca_ffs_init(FATFS *fs);

/*
 * Function: recursive_scan
 * --------------------------
 * Scan and print the contents of the SD card recursively.
 */
int recursive_scan(const char* path);

/*
 * Function: read_preset
 * --------------------------
 * Read a preset from the file `fname` into the given `readbuf`. Expect 
 * `buf_bytes` bytes.
 */
void read_preset(color_t readbuf[], unsigned int buf_bytes, const char *fname);

/*
 * Function: write_preset
 * --------------------------
 * Store a preset from the given `writebuf` of size `buf_bytes` to 
 * the given path `fname`.
 */
void write_preset(color_t writebuf[], unsigned int buf_bytes, const char *fname);

/*
 * Function: remove_preset
 * --------------------------
 * Remove the preset at `fname`.
 */
void remove_preset(const char *fname);

/*
 * Function: make_dir
 * --------------------------
 * Helper function that makes a directory `path` on the SD card.
 */
int make_dir(const char *path);

/*
 * Function: print_color_buf
 * --------------------------
 * Helper function that prints the contents of a color_t array `buf`, which
 * contains `num_entries` entries.
 */
void print_color_buf(color_t buf[], unsigned int num_entries);

#endif
