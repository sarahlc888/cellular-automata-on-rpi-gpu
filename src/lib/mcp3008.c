/* Avi Udash
 * 03/09/2022
 * Code for CS107E Final Project
 *
 * Library for initializing and reading from the mcp3008 ADC, using the SPI
 * library
 * Only supports single-ended input, not differential
 *
 * The implementation primarily used the MCP3008 datasheet:
 *    https://cdn-shop.adafruit.com/datasheets/MCP3008.pdf
 * Specifically, the pages 19-23 were incredibly useful
 *
 */
#include "../../include/mcp3008.h"
#include "printf.h"
#include "spi.h"

void mcp3008_init() { spi_init(SPI_CE0, 0); }

/*
 * Function: mcp3008_read
 * --------------------------
 *  @param channel_t channel: mcp3009 channel to read from
 *
 *  @return a 10-bit unsigned integer, since that is the max value
 *          that the mcp3008 can read
 */
unsigned int mcp3008_read(channel_t channel) {
  /* MCP3008 expects start bit to be high, followed by SGL/DIFF bit, followed by
   * three bits that determines which channel to read from. pg.19
   *
   *  - SGL/DIFF Bit determines whether the input is single-ended or
   *    differential. For this implementation, we assume input is single-ended.
   *
   *  - The next 3 bits are for the channel
   *
   *  - The last bit is to offset for 1 more clock to complete the sample
   *    period. The value of the bit doesn't matter.
   *
   * Once the data is transferred, MCP3008 sends back 1 NULL bit and 10 data
   * bits.
   */
  unsigned char d_in = 1 << 7 | channel << 4; // d_in of mcp3008, pg. 21

  /* tx[] is what is being transferred TO the mcp3008.
   * first, send 0b1 as start bit.
   * then, the d_in bits.
   * the next 8 bits don't matter, since we are just reading from mcp3008, so
   * just use 0.
   */
  unsigned char tx[] = {1, d_in, 0};

  // stores the mcp3008 result
  unsigned char rx[3];

  // transfer and read, with length of 3 8-bit characters
  spi_transfer(tx, rx, 3);

  rx[1] &= 0b00000011; // dont care about the first 6 bits, since we are still
                       // sending data during this time
  return (rx[1] << 8) | rx[2]; // mcp3008 sends most significant bit first
}
