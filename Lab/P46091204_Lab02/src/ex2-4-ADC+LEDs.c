/**
 * @file
 * Read ADC input
 *
 * Read the voltage at the potentiometer VR1 measured by the MCP3201 ADC
 * 
 * Then set LED by voltage
 *
 * Datasheet:
 * https://ww1.microchip.com/downloads/en/DeviceDoc/21290F.pdf
 */

#include <signal.h>
#include <time.h>
#include <stdbool.h>
#include "mraa.h"
#include "upboard_hat.h"
#include "util.h"

// ADC output resolution in bits
#define ADC_RESOLUTION 12
// Reference Voltage
#define VREF 3.3f

/** total number of leds (only 3 out of 8 outputs are used) */
#define LED_COUNT 3

mraa_gpio_context pin_clk, pin_data_ADC, pin_cs;

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}

int main() {
  // This line is used for resource collection. Ignore it.
  signal(SIGINT, int_handler);

  bool led_template[3][8] = {
    {0,1,0,0,0,0,0,0},          // LED2
    {0,0,1,0,0,0,0,0},          // LED3
    {0,0,0,1,0,0,0,0},          // LED4
  };
  bool led_status[8] = {0};

  // pin 12(CLK): clock pin
  pin_clk = mraa_gpio_init(UP_HAT_MCP3201_CLK);
  // pin 38(DOUT): SPI data pin 
  pin_data_ADC = mraa_gpio_init(UP_HAT_MCP3201_DOUT);
  // pin 40(CS/SHDN): pin_cs used to initiate communication with the device
  pin_cs = mraa_gpio_init(UP_HAT_MCP3201_CS);

  // pin 13(DS): serial data pin (input)
  mraa_gpio_context pin_data_LED = mraa_gpio_init(UP_HAT_74HC595_DS);
  // pin 15(STCP): when positive edge trigger, output shift register state
  mraa_gpio_context pin_en = mraa_gpio_init(UP_HAT_74HC595_STCP);
  // pin 16(SHCP): when positive edge trigger, shift parallel register & load form pin_DS
  mraa_gpio_context pin_sl = mraa_gpio_init(UP_HAT_74HC595_SHCP);

  if (!(pin_data_LED && pin_en && pin_sl)) {
    fprintf(stderr, "Failed to initalize GPIO. Did you run with sudo?\n");
    return EXIT_FAILURE;
  }

  mraa_gpio_dir(pin_clk, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_data_ADC, MRAA_GPIO_IN);
  mraa_gpio_dir(pin_cs, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_data_LED, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_en, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_sl, MRAA_GPIO_OUT);

  while (!stopped) {
    mraa_gpio_write(pin_cs, 0);  // SPI communication start
    delay_ns(1000);
    uint16_t data_ADC = 0;
    for (int SPICount = 0; SPICount < 15; SPICount++) {
      mraa_gpio_write(pin_clk, 0);
      delay_ns(1000);
      mraa_gpio_write(pin_clk, 1);
      if (SPICount >= 3) {  // first 3 cycles doesn't matter
        data_ADC = (data_ADC << 1) + mraa_gpio_read(pin_data_ADC);
      }
      delay_ns(1000);
    }
    mraa_gpio_write(pin_cs, 1);  //SPI communication end
    float voltage = VREF * data_ADC / (1 << ADC_RESOLUTION);
    printf("%fv\n", voltage);
    float percentage = voltage / 3.3;
    printf("Percentage = %f\n", percentage);

    if (percentage < 0.33){
        for (int i = 0; i < LED_COUNT; ++i) {  // for each LED
            led_status[i+1] = led_template[2][i+1]; 
        }
    }
    else if (percentage > 0.66){
        for (int i = 0; i < LED_COUNT; ++i) {  // for each LED
            led_status[i+1] = led_template[0][i+1];     
        }
    }
    else {
        for (int i = 0; i < LED_COUNT; ++i) {  // for each LED
            led_status[i+1] = led_template[1][i+1];
        }
    }

    mraa_gpio_write(pin_en, 1);  // unset pin_en to simulate positive edge later
    for (int i = 7; i >=0; i--) {   // for each bit from pin_DS (notice: order is reverse)
      // unset pin_clk to simulate positive edge later
      mraa_gpio_write(pin_sl, 1);
      // send led_status to data_pin.
      mraa_gpio_write(pin_data_LED, led_status[i]);
      // simulate positive edge to load & shift 1 bit data
      mraa_gpio_write(pin_sl, 0);
    }
    mraa_gpio_write(pin_en, 0);  // set pin_en: output register state to light up LED.

    delay_ms(10);
  }
  
  /* release resource section */
  mraa_gpio_close(pin_data_ADC);
  mraa_gpio_close(pin_cs);
  mraa_gpio_close(pin_clk);

/* release resource section */
  mraa_gpio_write(pin_en, 0);
  for (int i = 7; i >=0; i--) {
    mraa_gpio_write(pin_sl, 0);
    mraa_gpio_write(pin_data_LED, 0);
    mraa_gpio_write(pin_sl, 1);
  }
  mraa_gpio_write(pin_en, 1);
  mraa_gpio_close(pin_data_LED);
  mraa_gpio_close(pin_en);
  mraa_gpio_close(pin_sl);
}
