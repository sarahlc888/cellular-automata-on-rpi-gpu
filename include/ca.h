#ifndef CA_H
#define CA_H

#include "gl.h"

void ca_init(unsigned int ca_mode, 
    unsigned int screen_width, unsigned int screen_height, 
    color_t foreground, color_t background,
    unsigned int update_delay);

void ca_run(void);

#endif
