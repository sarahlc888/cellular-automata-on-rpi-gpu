#ifndef RWCA_H
#define RWCA_H

void ca_ffs_init(void);
int recursive_scan(const char* path);
void read_preset(color_t readbuf[], unsigned int buf_bytes, const char *fname);
void write_preset(color_t writebuf[], unsigned int buf_bytes, const char *fname);
void remove_preset(const char *fname);
void run_tests(void);

#endif
