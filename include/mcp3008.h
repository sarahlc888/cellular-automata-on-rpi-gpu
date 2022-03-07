#include "spi.h"

enum channel_t{
  CH0 = 0b000,
  CH1 = 0b001,
  CH2 = 0b010,
  CH3 = 0b011,
  CH4 = 0b100,
  CH5 = 0b101,
  CH6 = 0b110,
  CH7 = 0b111
} ;

void mcp3008_init();
unsigned int mcp3008_read(enum channel_t channel);
