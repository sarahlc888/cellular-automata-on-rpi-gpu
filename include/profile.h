#ifndef PROFILE_H
#define PROFILE_H

/*
 * Module to run profiler extension (implemented during Assignment 7) 
 * via function calls.
 * 
 * Sarah Chen
 */

/*
 * `profile_init`
 *
 * Initialize the profiler by setting up interrupts.
 */
void profile_init(void);
/*
 * `profile_on`
 *
 * Turn the profiler on. Do nothing if the profiler is already on.
 */
int profile_on(void);
/*
 * `profile_off`
 *
 * Turn the profiler off and print a summary of most common instructions.
 * Do nothing if the profiler is already off.
 */
int profile_off(void);

#endif
