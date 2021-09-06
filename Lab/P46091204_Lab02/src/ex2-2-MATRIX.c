/**
 * Basic LED matrix display with MAX7219
 *
 *
 * Datasheet:
 * https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
 */

#include <signal.h>
#include <stdint.h>

#include "mraa.h"
#include "upboard_hat.h"
#include "util.h"

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}

mraa_gpio_context pin_load, pin_din, pin_clk;

/**
 * Send a byte over the DIN pin
 */
void send_byte(uint8_t d) {
  for (int i = 7; i >= 0; --i) {
    mraa_gpio_write(pin_clk, 0);
    delay_ns(1000);
    mraa_gpio_write(pin_din, (d >> i) & 1u);
    delay_ns(1000);
    mraa_gpio_write(pin_clk, 1);
    delay_ns(1000);
  }
}

/**
 * Write data to the register at addr
 */
void write_reg(uint8_t addr, uint8_t data) {
  mraa_gpio_write(pin_clk, 1);
  mraa_gpio_write(pin_load, 0);
  send_byte(addr);
  send_byte(data);
  mraa_gpio_write(pin_clk, 0);
  mraa_gpio_write(pin_load, 1);
  delay_ns(1000);
}

/**
 * Setup the MAX7219 driver
 */
void init_matrix() {
  // Refer to the datasheet for the meaning of these numbers

  write_reg(0x0c, 0x01);  // leave shutdown mode
  write_reg(0x0f, 0x00);  // turn off display test
  write_reg(0x09, 0x00);  // set decode mode: no decode
  write_reg(0x0b, 0x07);  // set scan limit to full
  write_reg(0x0a, 0x01);  // set brightness (duty cycle = 3/32)
}

int main() {
  pin_load = mraa_gpio_init(UP_HAT_MAX7219_LOAD);
  pin_din = mraa_gpio_init(UP_HAT_MAX7219_DIN);
  pin_clk = mraa_gpio_init(UP_HAT_MAX7219_CLK);

  if (!(pin_load && pin_din && pin_clk)) {
    fprintf(stderr, "Failed to initalize GPIO. Did you run with sudo?\n");
    return EXIT_FAILURE;
  }

  mraa_gpio_dir(pin_load, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_din, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_clk, MRAA_GPIO_OUT);

  init_matrix();

  // Each bit corresponds to an LED in a row

  uint8_t row_pattern = 0xFF;

  signal(SIGINT, int_handler);
  while (!stopped) {
    
    // To set the pattern of the nth row (1-indexed), write to register n the
    // 8-bit pattern
    for (int i = 1; i <= 8; ++i) {
      printf("write_reg(%d, %d)\n", i, row_pattern);
      write_reg(i, row_pattern);
    }

    // rotate pattern left
    row_pattern = row_pattern > 1 ? row_pattern >> 1 : 0xFF;

    delay_ms(100);
  }
  
  /* release resource section */
  for (int i = 1; i <= 8; ++i) {
    write_reg(i, 0x00);
  }
  mraa_gpio_close(pin_load);
  mraa_gpio_close(pin_din);
  mraa_gpio_close(pin_clk);
}
