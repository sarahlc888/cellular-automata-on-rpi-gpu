#include "../../include/mcp3008.h"
#include "printf.h"
#include "spi.h"
#include "timer.h"

int main() {
  mcp3008_init();
  // unsigned char tx = 1 << 5 | 1 << 4 | 0b010 << 1 | 1;
  while (1) {
    int left_pot = mcp3008_read(CH0);
    // int right_pot = 0;
    int right_pot = mcp3008_read(CH1);
    printf("left pot: %d, right pot: %d\n", left_pot, right_pot);
    timer_delay(1);
  }
  // printf("workd\n");

  return 0;
}
