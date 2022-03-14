/* Code taken directly from past CS107E project ahconkey-JoshFrancisCodes-project */

#define V3D_BASE 0x20c00000 // Source Broadcom VC4 doc, page 110 (https://docs.broadcom.com/doc/12358545)

// Address found on pages 90-91 of Broadcom Manual
static const unsigned PROGRAM = V3D_BASE + 0x430; // address to write address of QPU program
static const unsigned UNIFORMS = V3D_BASE + 0x434; // address to write address of uniforms
static const unsigned ULENGTH = V3D_BASE + 0x438; // address to write length of uniforms
static const unsigned V3D_SRQCS = V3D_BASE + 0x43c; // info about req counts
static const unsigned V3D_DBQITE = 0x34; // interrupt enables

/*
 * 'qpu_init'
 *
 * Enable the qpu and set the clock rate to setup qpu functionality.
 *
 * @return  int value where 0 is success
 */
int qpu_init(void);

/*
 * 'qpu_malloc'
 *
 * This function calls both mem_alloc and mem_lock to allocate 'size'
 * amount of space on the GPU, and then returns a handle to the memory
 * so that it can be handled by the CPU.
 *
 * @param size   number of words (4 bytes) to allocate on GPU
 *
 * @return   unsigned int representing the handle
 */
unsigned qpu_malloc(unsigned size);

/*
 * 'qpu_free'
 *
 * This function calls both mem_unlock and mem_free to free up the
 * allocated memory on the GPU.
 *
 * @param handle   unsigned int handle for memory on GPU
 *
 * @return   unsigned int value where 0 is success
 */
unsigned qpu_free(unsigned handle);

/*
 * 'qpu_run'
 *
 * Takes in qpu assembly code and an array of uniforms and runs the
 * code on the qpu with the uniforms passed as parameters. QPU must
 * be initialized prior to calling this function.
 *
 * @param code            pointer to array containing qpu assembly code
 * @param code_length     the size of code array
 * @param uniforms        pointer to array containing uniforms
 * @param uniforms_count  the size of uniforms array
 *
 * @return   int value where 0 is success
 */
int qpu_run(unsigned *code, unsigned code_length, unsigned *uniforms, unsigned uniforms_count);

/*
 * 'qpu_request_count'
 *
 * Checks the V3D_SRQCS register to see how many QPU requests have been made
 *
 * @return   unsigned char representing how many requests have been made
 */
unsigned char qpu_request_count();

/*
 * 'qpu_complete_count'
 *
 * Checks the V3D_SRQCS register to see how many QPU requests have been
 * completed
 *
 * @return   unsigned char representing how many requests have been completed
 */
unsigned char qpu_complete_count();

/*
 * 'qpu_reset_status'
 *
 * Resets the QPU to be ready to run again by setting bits in the V3D_SRQCS
 * register
 */
void qpu_reset_status();

/*
 * 'qpu_print_status'
 *
 * Prints out the current status of the V3D_SRQCS register to console. 
 * Useful for debugging.
 */
void qpu_print_status();

/*
 * 'qpu_enable_interrupts'
 *
 * Enables interrupts for the QPU. Not currently used by anything, but
 * could be useful for future projects because QPU assembly can react
 * pretty well to interrupts.
 */
void qpu_enable_interrupts();
