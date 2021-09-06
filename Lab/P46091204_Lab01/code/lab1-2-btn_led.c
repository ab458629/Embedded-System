/**
 * @file
 * Sync a switch and an LED
 *
 * Turn on LED1 when SW1 is pressed
 * Turn off LED1 when SW1 is released
 */

#include <signal.h>
#include <stdio.h>
#include <time.h>

#include "mraa.h"
#include "upboard_hat.h"

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}

int main() {
  // pin 18: gpio pin with led1
  mraa_gpio_context gpio_led = mraa_gpio_init(UP_HAT_LED1);
  // pin 22: gpio pin with sw1
  mraa_gpio_context gpio_btn = mraa_gpio_init(UP_HAT_SW1);

  if (!(gpio_led && gpio_btn)) {
    fprintf(stderr, "Failed to initalize GPIO. Did you run with sudo?\n");
    return EXIT_FAILURE;
  }

  mraa_gpio_dir(gpio_led, MRAA_GPIO_OUT);  // make pin 18 an output pin
  mraa_gpio_dir(gpio_btn, MRAA_GPIO_IN);   // make pin 22 an input pin

  signal(SIGINT, int_handler);

  while (!stopped) {
  
    /* modify section below this line */
    
    // read gpio_btn's value
    // write proper value to gpio_led. Proper value is referenced from gpio_btn's value
    if (mraa_gpio_read(gpio_btn) == 0)  //The pin value becomes 0 when button is pressed
    {
      if (mraa_gpio_read(gpio_led))         // if high voltage
        mraa_gpio_write(gpio_led, 0);
      else                              // else if low voltage
        mraa_gpio_write(gpio_led, 1);
      
    }
    /* modify section above this line */
    
  }
  mraa_gpio_close(gpio_led);
  mraa_gpio_close(gpio_btn);
}
