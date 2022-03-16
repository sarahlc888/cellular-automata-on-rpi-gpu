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

// initialize the file system
void ca_ffs_init(FATFS *fs);
// scan the contents of the SD card
int recursive_scan(const char* path);
// read a preset from the file name into the given readbuf
void read_preset(color_t readbuf[], unsigned int buf_bytes, const char *fname);
// store a preset from the given writeful to the given file name
void write_preset(color_t writebuf[], unsigned int buf_bytes, const char *fname);
// remove the preset at the given file name
void remove_preset(const char *fname);
// make a directory on the SD card
int make_dir(const char *path);

#endif
