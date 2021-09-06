/**
 * @file
 * Blink LED
 *
 * Blink LED1 at 1Hz
 */

#include <signal.h>
#include <stdio.h>
#include "util.h"
#include "mraa.h"
#include "upboard_hat.h"

mraa_gpio_context gpio;  // declare gpio pin

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}

int main() {
  gpio = mraa_gpio_init(UP_HAT_LED1);  // pin 18: gpio pin with led1
  if (!gpio) {
    fprintf(stderr, "Failed to initalize GPIO. Did you run with sudo?\n");
    return EXIT_FAILURE;
  }
  mraa_gpio_dir(gpio, MRAA_GPIO_OUT);  // assign pin 18 to be an output pin

  signal(SIGINT, int_handler);

  while (!stopped) {
  
    /* modify section below this line */
    mraa_gpio_write(gpio, 1);
    // turn on the led for 1 second
    delay_seconds(1);  //sleep 1 sec
    // turn off the led for 1 second
    mraa_gpio_write(gpio, 0);
    delay_seconds(1);  //sleep 1 sec
    
    /* modify section above this line */
    
  }

  mraa_gpio_close(gpio);
}
