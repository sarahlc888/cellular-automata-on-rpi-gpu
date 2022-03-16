/* Avi Udash
 * 03/09/2022
 * Code for CS107E Final Project
 *
 * Library for initializing and reading from the mcp3008 ADC, using the SPI library
 * Only supports single-ended input, not differential
 *
 * The implementation primarily used the MCP3008 datasheet: 
 *    https://cdn-shop.adafruit.com/datasheets/MCP3008.pdf
 * Specifically, the pages 19-23 were incredibly useful
 *
 * More info on specifics of implementation is on 'mcp3008.c' file
 */
#include "spi.h"


/* 
 * MCP3008 supports 8 channels.
 * table on pg. 19
 */
typedef enum channels{
  CH0 = 0b000,
  CH1 = 0b001,
  CH2 = 0b010,
  CH3 = 0b011,
  CH4 = 0b100,
  CH5 = 0b101,
  CH6 = 0b110,
  CH7 = 0b111
} channel_t;

/* initialize the mcp3008 on SPI CE0. 
 * Doesn't expect SPI to be previously init
*/ 
void mcp3008_init();

/*
 * Function: mcp3008_read
 * --------------------------
 *  @param channel_t channel: mcp3009 channel to read from
 *
 *  @return a 10-bit unsigned integer, since that is the max value
 *          that the mcp3008 can read
 */
unsigned int mcp3008_read(channel_t channel);
