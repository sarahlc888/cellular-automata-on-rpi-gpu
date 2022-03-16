/* 
 * Sarah Chen
 * 03/10/2022
 * Code for CS107E Final Project
 * 
 * This code is adapted from the extension for CS107E Assignment 7.
 * 
 * This module implements a profiler that measures where a program spends most 
 * of its time. It interrupts the program to sample the pc at regular intervals.
 */

#include "../../include/profile.h"
#include "uart.h"
#include "strings.h"
#include "malloc.h"
#include "timer.h"
#include "armtimer.h"
#include "interrupts.h"
#include "backtrace.h"
#include "printf.h"

#define PROFILE_INTERVAL 500 // interval of 500 microseconds between sampling for `profile`
#define PROFILER_N 20 // the number of top instructions to show 
#define PROLOGUE_INSTR 0xe1a0c00d // instruction at start of each function prologue

// constants
extern int __text_end__; // the end of the text section in memory, as specified by memmap
static int text_start = 0x8000; // start of text section
static int total_instructions; // number of instructions in text/code section

// track whether profiler is on or off (default off)
static unsigned int profiler_status = 0; 
// pointer to counters for all text instructions 
static unsigned int *profiler_ct;

/*
 * Function: instr_addr_to_index
 * --------------------------
 * Handler function to convert an address (of an instruction in the text/code section) 
 * into its index as the `n`th instruction of the program. It expects the given address
 * to correspond to a sampled pc. Note that the pc value of executing instruction is 
 * offset by 8.
 */
static unsigned int instr_addr_to_index(unsigned int addr)
{
    if (addr < text_start + 8) {
        return 0;
    }
    // convert to instruction index
    return (addr - 8 - text_start) / 4; 
}

/*
 * Function: instr_index_to_addr
 * --------------------------
 * Handler function to convert an instruction index (its position as the `n`th 
 * instruction of the program) into its address (in the text/code section). 
 */
static int *instr_index_to_addr(unsigned int ind)
{
    ind *= 4;
    ind += text_start;
    return (int *) ind;
}

/*
 * Function: handle_profiler_sample
 * --------------------------
 * Handler function to process armtimer interrupts. It maintains and updates the 
 * global array of counters `profiler_ct`, which has one entry for each instruction 
 * in the text section.
 */
static void handle_profiler_sample(unsigned int pc, void *aux_data)
{
    // prevent endless retriggering
    armtimer_check_and_clear_interrupt();

    // get counter pointer and update count
    unsigned int instr_ind = instr_addr_to_index(pc);
    profiler_ct[instr_ind] = profiler_ct[instr_ind] + 1; 
}

/*
 * Function: shift_down
 * --------------------------
 * This function copies all elements from `ind` to `arr_size - 2` back one position, to
 * `ind + 1` to `arr_size - 1`.
 */
static void shift_down(int *arr, int arr_size, int ind) 
{
    if (arr_size == 0) {
        return;
    }
    for (int i = arr_size - 1; i > ind; i--) {
        arr[i] = arr[i - 1];
    }
}

/*
 * Function: find_insertion_pos
 * --------------------------
 * This function finds the position to insert `elem` in `arr` that maintains 
 * descending sorted order.
 */
static int find_insertion_pos(int *arr, int arr_size, int elem) 
{
    // start of array: element is greater than all array elements
    if (elem >= arr[0]) {
        return 0;
    }

    // end: element is less than all array elements
    else if (elem <= arr[arr_size - 1]) {
        return arr_size;
    }

    // middle: element fits between array elements
    for (int i = 0; i < arr_size - 1; i++) {
        if (arr[i] >= elem && elem >= arr[i + 1]) {
            return i + 1;
        }
    }
    
    // default to adding element to the end
    return arr_size; 
}

/*
 * Function: find_highest_counts
 * --------------------------
 * This function scans the counter values in `profiler_ct` to find the entries 
 * with the `n` highest counts. It stores the instruction indices in the given
 * array `top_instr` and the corresponding counts in `vals`.
 * 
 * It returns the number of instructions placed into `top_instr`.
 */
static int find_highest_counts(int *top_instr, int *vals, int n)
{
    // number of entries in `top_instr` filled so far
    int top_size = 0;

    // find the entries with the highest counts
    for (int i = 0; i < total_instructions; i++) {
        // skip instructions that were not recorded
        if (profiler_ct[i] == 0) {
            continue;
        }
        
        // determine where within `top_instr` the given instruction falls
        int insertion_ind = find_insertion_pos(vals, top_size, profiler_ct[i]);

        // ignore if the count is too small to be considered
        if (insertion_ind >= top_size && top_size == n) {
            continue;
        } 

        // add the instruction and value to `top_instr` and `vals`
        shift_down(top_instr, top_size, insertion_ind); // move elements
        top_instr[insertion_ind] = i; // store index
        shift_down(vals, top_size, insertion_ind); // move elements
        vals[insertion_ind] = profiler_ct[i]; // store index

        // increment size unless maximum size has been reached
        if (top_size < n) {
            top_size++;
        }
    }

    return top_size;
}

/*
 * Function: find_function_start
 * --------------------------
 * This function finds the function that an instruction at the pointer `instr_address` 
 * belongs to and backs up through memory to return the start address of that function.
 * To do so, it looks for the expected function prologue, which should be preceded
 * by a word with 0xff000000 set (assuming that the function names are embedded).
 */
static uintptr_t find_function_start(int *instr_address)
{
    // work backward in memory until encountering a function prologue
    while ((uintptr_t) instr_address > text_start && *instr_address != PROLOGUE_INSTR) {
        instr_address = instr_address - 1;
    }
    uintptr_t instr_addr = (uintptr_t) instr_address;

    // ensure that the function name is embedded before the detected prologue
    // (the same instruction may have arisen from a non-function-prologue event)
    if (instr_addr != text_start) {
        uintptr_t func_name = *(int *)(instr_addr - 4);
        if ((0xff000000 & func_name) == 0xff000000) {
            return instr_addr;
        }
    }

    // if not, issue a warning but still proceed
    printf("warning: function name not present before detected function start\n");
    return instr_addr;
}

/*
 * Function: print_top_instr
 * --------------------------
 * This function prints the `n` instructions with the highest counter values, where 
 * instruction indices are specified in `top_instr_ind` and corresponding counter values 
 * are in `vals`.
 * 
 * It also accepts the argument `top_size`, which specifies the number of instructions
 * stored within `top_instr_ind`. If `top_size` < `n`, this limits the number of entries
 * printed.
 */
static void print_top_instr(int *top_instr_ind, int *vals, int top_size, int n)
{
    printf("  Counts  |  Function    [pc]\n-----------------------------\n");
    for (int i = 0; i < n && i < top_size; i++) {
        int cur_ind = top_instr_ind[i]; // index of the top PC entry (nth instruction)
        int *cur_addr = instr_index_to_addr(cur_ind);

        // find first instruction of current function
        uintptr_t func_start = find_function_start(cur_addr); 

        // Find the function name that each high-count instruction belongs to
        const char* func_name = name_of(func_start); 

        printf("    %d  |  %s+%d [0x%04x]\n", profiler_ct[top_instr_ind[i]], func_name, (uintptr_t)cur_addr - func_start, (uintptr_t)cur_addr);
    }
}

/*
 * Function: profile_on
 * --------------------------
 * Turn the profiler on and begin counting the instructions that the `pc` is on 
 * during armtimer interrupts. 
 * 
 * Do nothing if profiler is already on.
 */
int profile_on(void)
{
    if (profiler_status == 1) { 
        return 0;
    }

    // turn on profiler
    profiler_status = 1;

    // use __text_end__ to compute space needed
    total_instructions = ((uintptr_t) &__text_end__ - (uintptr_t) text_start) / 4;
    profiler_ct = malloc(4 * total_instructions); // 4 byte integer for each instruction

    printf("total instructions, starting at 0x8000: %d\n", total_instructions);

    // zero-out all counts
    for (int i = 0; i < total_instructions; i++) {
        profiler_ct[i] = 0;
    }

    armtimer_enable(); // start timer 

    return 0;
}

/*
 * Function: profile_off
 * --------------------------
 * Turn the profiler off (disable armtimer) and print a summary of the 
 * instructions with the highest counts (i.e. that dominated the program's 
 * run time).
 * 
 * Do nothing if profiler is already off.
 */
int profile_off(void)
{
    if (profiler_status == 0) {
        return 0;
    }

    // turn off profiler
    profiler_status = 0;
    armtimer_disable(); // stop timer 

    // print a list of the 20 instructions with the highest counts
    int *top_instr = malloc(PROFILER_N * 4); // instruction indices
    int *top_cts = malloc(PROFILER_N * 4); // store corresponding profiler counts
    int num_instr = find_highest_counts(top_instr, top_cts, PROFILER_N);
    print_top_instr(top_instr, top_cts, num_instr, PROFILER_N);
    free(top_instr);
    free(profiler_ct);

    return 0;
}

/*
 * Function: profile_init
 * --------------------------
 * Run necessary initialization steps. Run this function before any others in the module.
 * The function initializes the arm timer and enables interrupts to allow the profiler
 * to function.
 */
void profile_init(void)
{
    armtimer_init(PROFILE_INTERVAL); // initialize timer for profiler
    armtimer_enable_interrupts(); 
    // register handler with dispatcher 
    interrupts_register_handler(INTERRUPTS_BASIC_ARM_TIMER_IRQ, handle_profiler_sample, NULL);
    interrupts_enable_source(INTERRUPTS_BASIC_ARM_TIMER_IRQ); // enable interrupt source
}