/**
 * Control multiple with via the 74HC595 shift register
 *
 * Turn on LEDs LED2, LED3 and LED4 sequentially with 74HC595
 *
 * Datasheet:
 * https://www.diodes.com/assets/Datasheets/74HC595.pdf
 */

#include <signal.h>
#include <stdio.h>
#include <time.h>
#include "mraa.h"
#include "upboard_hat.h"
#include "util.h"

/** total number of leds (only 3 out of 8 outputs are used) */
#define LED_COUNT 3

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}
int main() {
  int led_template[3][8] = {
    {0,1,0,0,0,0,0,0},
    {0,0,1,0,0,0,0,0},
    {0,0,0,1,0,0,0,0},
  };
  int led_status[8] = {0};
  // pin 13(DS): serial data pin (input)
  mraa_gpio_context pin_data = mraa_gpio_init(UP_HAT_74HC595_DS);
  // pin 15(STCP): when positive edge trigger, output shift register state
  mraa_gpio_context pin_en = mraa_gpio_init(UP_HAT_74HC595_STCP);
  // pin 16(SHCP): when positive edge trigger, shift parallel register & load form pin_DS
  mraa_gpio_context pin_sl = mraa_gpio_init(UP_HAT_74HC595_SHCP);

  if (!(pin_data && pin_en && pin_sl)) {
    fprintf(stderr, "Failed to initalize GPIO. Did you run with sudo?\n");
    return EXIT_FAILURE;
  }
  mraa_gpio_dir(pin_data, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_en, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_sl, MRAA_GPIO_OUT);

  int t = 0;  // time variable
  signal(SIGINT, int_handler);

  while (!stopped) {
    for (int i = 0; i < LED_COUNT; ++i) {  // for each LED
      led_status[i+1] = led_template[t][i+1];
    }

    // pin_en = STCP, pin_sl = SHCP

    mraa_gpio_write(pin_en, 1);  // unset pin_en to simulate positive edge later
    for (int i = 7; i >=0; i--) {   // for each bit from pin_DS (notice: order is reverse)
      // unset pin_clk to simulate positive edge later
      mraa_gpio_write(pin_sl, 1);
      // send led_status to data_pin.
      mraa_gpio_write(pin_data, led_status[i]);
      // simulate positive edge to load & shift 1 bit data
      mraa_gpio_write(pin_sl, 0);
    }
    mraa_gpio_write(pin_en, 0);  // set pin_en: output register state to light up LED.
    delay_seconds(1);
    // t = {0, 1, 2} loop
    t++;
    if (t >= LED_COUNT)
      t = 0;
  }
  
  /* release resource section */
  mraa_gpio_write(pin_en, 0);
  for (int i = 7; i >=0; i--) {
    mraa_gpio_write(pin_sl, 0);
    mraa_gpio_write(pin_data, 0);
    mraa_gpio_write(pin_sl, 1);
  }
  mraa_gpio_write(pin_en, 1);
  mraa_gpio_close(pin_data);
  mraa_gpio_close(pin_en);
  mraa_gpio_close(pin_sl);
}
