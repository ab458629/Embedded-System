/**
 * @file
 * Control LED brightness with PWM
 *
 * Set the LED brigthness to resemble a sawtooth wave pattern with UP Board's
 * builtin PWM
 */

#include <signal.h>
#include <stdio.h>

#include "mraa.h"
#include "upboard_hat.h"
#include "util.h"

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}

int main() {
  // pin 32: pwm pin with LED5
  mraa_pwm_context pin_pwm = mraa_pwm_init(UP_HAT_LED5);
  mraa_pwm_period_us(pin_pwm, 200);  // set pwm frequency to 5000hz
  mraa_pwm_enable(pin_pwm, 1);       // enable pwm pin

  float duty_cycle = 0.0f;

  signal(SIGINT, int_handler);
  while (!stopped) {
    printf("DUTY CYCLE: %f\n", duty_cycle);
    mraa_pwm_write(pin_pwm, duty_cycle);  // change LED duty cycle

    duty_cycle = duty_cycle + 0.01f;  // increment the duty cycle
    if (duty_cycle >= 1.0f) {
      duty_cycle = 0.0f;
    }
    delay_ms(10);
  }
  /* release resource section */
  mraa_pwm_write(pin_pwm, 0.0);
  mraa_pwm_enable(pin_pwm, 0);
  mraa_pwm_close(pin_pwm);
}
