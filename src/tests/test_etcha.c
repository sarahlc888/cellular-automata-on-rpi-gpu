#include "../../include/mcp3008.h"
#include "gl.h"
#include "gpio.h"
#include "gpio_extra.h"
#include "gpio_interrupts.h"
#include "interrupts.h"
#include "printf.h"
#include "ringbuffer.h"
#include "timer.h"
#include "uart.h"

static bool is_drawing = false;
static unsigned button_pin = 21;

void handle_button(unsigned pc, void *aux_data) {
  gpio_clear_event(button_pin);
  is_drawing = !is_drawing;
}

int main() {
  mcp3008_init();

  gpio_init();
  gpio_set_pullup(button_pin);
  interrupts_init();
  gpio_interrupts_init();
  gpio_enable_event_detection(21, GPIO_DETECT_FALLING_EDGE);
  gpio_interrupts_register_handler(button_pin, handle_button, NULL);
  gpio_interrupts_enable();
  interrupts_global_enable();

  gl_init(100, 100, GL_DOUBLEBUFFER);
  gl_clear(GL_BLACK);

  while (true) {
    int x = mcp3008_read(CH0) / 10;
    int y = mcp3008_read(CH1) / 10;
    printf("x: %d, y: %d\n", x, y);
    if (is_drawing) {
      gl_draw_pixel(x, y, GL_WHITE);
      gl_swap_buffer();
      gl_draw_pixel(x, y, GL_WHITE);
    }
    // timer_delay(1);
  }

  uart_putchar(EOT);

  return 0;
}
