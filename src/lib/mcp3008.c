#include "../../include/mcp3008.h"
#include "printf.h"
#include "spi.h"

void mcp3008_init() { spi_init(SPI_CE0, 0); }

unsigned int mcp3008_read(enum channel_t channel) {
  /* MCP3008 expects start bit to be high, followed by SGL/DIFF bit, followed by
   * three bits that determines which channel to read from.
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
  unsigned char d_in = 1 << 7 | channel << 4;
  unsigned char tx[] = {1, d_in, 0};
  unsigned char rx[3];

  spi_transfer(tx, rx, 3);

  rx[1] &= 0b00000011; // dont care about the first 6 bits, since we are still
                       // sending data during this time
  return (rx[1] << 8) | rx[2];
}
