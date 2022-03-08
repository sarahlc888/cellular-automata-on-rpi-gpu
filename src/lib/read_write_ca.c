/*
 * Simple client example using fatfs to access filesystem on SD card
 */
#include "printf.h"
#include "strings.h"
#include "uart.h"
#include "gl.h"
#include "ff.h"

// copied from $CS107E/examples/sd_fatfs
static const char *name_of(FILINFO *f)
{
    return ((f->fattrib & AM_LFN) || (f->lfname && *f->lfname)) ? f->lfname : f->fname;
}

// copied from $CS107E/examples/sd_fatfs
static bool is_dir(FILINFO *f)
{
   return f->fattrib & AM_DIR;
}

// copied from $CS107E/examples/sd_fatfs
static void print_fileinfo(FILINFO *f, const char *dir)
{
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    int year = f->fdate >> 10;
    int mon = (f->fdate >> 5) & 0xf;
    int day = f->fdate & 0x1f;
    int hour = f->ftime >> 11;
    int min = (f->ftime >> 5) & 0x3f;
    //int sec = f->ftime & 0x3f;

    printf("%8d %s %2d 20%02d %2d:%02d %s/%s%s\n", 
        (int)f->fsize, months[mon-1], day, year, hour, min,  dir, name_of(f), is_dir(f) ? "/" :"");
}

// copied from $CS107E/examples/sd_fatfs
int recursive_scan(const char* path)
{
    FRESULT res;
    DIR dir;
    FILINFO fi;
    int count = 0;

    char lfnbuf[128];  // used to capture long file name
    fi.lfsize = sizeof lfnbuf;
    fi.lfname = lfnbuf;

    res = f_opendir(&dir, path);    
    if (res != FR_OK) {
        printf("opendir error %s\n", ff_err_msg(res));
        return -1;
    }

    while (1) {
        res = f_readdir(&dir, &fi);
        if (res != FR_OK) {
            printf("readdir error %s\n", ff_err_msg(res));
            return -1;
        }
        if (fi.fname[0] == 0) break;  // no more entries
        if (name_of(&fi)[0] == '.') continue; // skip hidden

        if (is_dir(&fi)) {
            char full_path[1024];
            snprintf(full_path, sizeof full_path, "%s/%s", path, fi.fname);
            count += recursive_scan(full_path);                
        } else {
            print_fileinfo(&fi, path);
            count++;            
        }
    }
    f_closedir(&dir);
    return count;
}

// print function for debugging purposes
static void print_color_buf(color_t buf[], unsigned int num_entries)
{
    for (int i = 0; i < num_entries; i++) {
        printf("%x ", buf[i]);
    }
    printf("\n");
}

// delete the specified file
void remove_preset(const char *fname)
{
    FRESULT res;

    // remove the file
    // see http://elm-chan.org/fsw/ff/doc/unlink.html
    // for information about the f_unlink function
    res = f_unlink(fname);
    if (res != FR_OK) {
        printf("Could not remove file %s. Error: %s\n",fname, ff_err_msg(res));
        return;
    }
    printf("Removed file %s.\n",fname);
}

// TODO: make read and write less verbose in terms of printing
// write the given preset array to the file name
void write_preset(color_t writebuf[], unsigned int buf_bytes, const char *fname)
{
    FRESULT res;

    // create a test file in the root directory
    // see http://elm-chan.org/fsw/ff/doc/open.html
    // for information about the f_open function
    printf("Creating file %s.\n", fname); 
    FIL fp;
    res = f_open(&fp, fname, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        printf("Could not create file %s. Error: %s\n",fname, ff_err_msg(res));
        return;
    }

    // write something to the file
    // see http://elm-chan.org/fsw/ff/doc/write.html
    // for information about the f_write function
    unsigned int nwritten;
    res = f_write(&fp, writebuf, buf_bytes, &nwritten);
    if (res != FR_OK || nwritten != buf_bytes) {
        printf("Could not write to file %s. Error: %s\n", fname, ff_err_msg(res));
        f_close(&fp);
        return;
    }
    printf("Wrote %d bytes to file %s:\n---\n", nwritten, fname); 
    // print_color_buf(writebuf, buf_bytes / 4);
    printf("---\n"); 
    f_close(&fp);   // close file (should commit to media)
}

// read the given preset array from the file name
void read_preset(color_t readbuf[], unsigned int buf_bytes, const char *fname)
{
    FRESULT res;
    FIL fp;

    // we are going to read the entire file, so we can use the f_stat function 
    // to get the file length, and we can also use this to determine if the file exists
    FILINFO fi;
    res = f_stat(fname,&fi);
    if (res != FR_OK) {
        printf("Could not stat file %s. Error: %s\n", fname, ff_err_msg(res));
        return;
    } else {
        printf("ls %s (file size should be %d bytes, date/time is bunk)\n", fname, buf_bytes);
        print_fileinfo(&fi, "");
    }

    // open the file up again to read it
    res = f_open(&fp, fname, FA_READ);
    if (res != FR_OK) {
        printf("Could not open file %s for reading. Error: %s\n",fname, ff_err_msg(res));
        return;
    }
    // read the entire file into a buffer
    // see http://elm-chan.org/fsw/ff/doc/read.html
    // for information about the f_read function
    unsigned int nread;
    res = f_read(&fp, readbuf, buf_bytes, &nread);
    if (res != FR_OK || nread != buf_bytes) {
        printf("Could not read from file %s. Error: %s\n",fname, ff_err_msg(res));
        return;
    }
    printf("Read %d bytes from file %s:\n---\n", nread, fname); 
    // print_color_buf(readbuf, buf_bytes / 4);
    printf("---\n"); 


}

int make_dir(const char *path)
{
    FRESULT res;
    res = f_mkdir(path);       
    if (res != FR_OK) {
        printf("mkdir error %s\n", ff_err_msg(res));
        printf("%s", path);
        return -1;
    }      
    return 0;   
}

// initialize file system
void ca_ffs_init(FATFS *fs)
{
    FRESULT res = f_mount(fs, "", 1);
    if (res != FR_OK) {
        printf("Could not mount internal SD card. Error: %s\n", ff_err_msg(res));
        return;
    }

}

// adapted from $CS107E/examples/sd_fatfs
void run_tests(void) 
{
    uart_init();
    printf("Starting libpisd.a test\n");

    printf("initializing\n");

    // TODO: does not work when using ca_ffs_init, probably
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
