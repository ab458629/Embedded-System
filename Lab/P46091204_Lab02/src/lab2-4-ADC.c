/**
 * @file
 * Read ADC input
 *
 * Read the voltage at the potentiometer VR1 measured by the MCP3201 ADC
 *
 * Datasheet:
 * https://ww1.microchip.com/downloads/en/DeviceDoc/21290F.pdf
 */

#include <signal.h>
#include <time.h>

#include "mraa.h"
#include "upboard_hat.h"
#include "util.h"

// ADC output resolution in bits
#define ADC_RESOLUTION 12
// Reference Voltage
#define VREF 3.3f

mraa_gpio_context pin_clk, pin_data, pin_cs;

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}

int main() {
  // This line is used for resource collection. Ignore it.
  signal(SIGINT, int_handler);

  // pin 12(CLK): clock pin
  pin_clk = mraa_gpio_init(UP_HAT_MCP3201_CLK);
  // pin 38(DOUT): SPI data pin 
  pin_data = mraa_gpio_init(UP_HAT_MCP3201_DOUT);
  // pin 40(CS/SHDN): pin_cs used to initiate communication with the device
  pin_cs = mraa_gpio_init(UP_HAT_MCP3201_CS);

  mraa_gpio_dir(pin_clk, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_data, MRAA_GPIO_IN);
  mraa_gpio_dir(pin_cs, MRAA_GPIO_OUT);

  while (!stopped) {
    mraa_gpio_write(pin_cs, 0);  // SPI communication start
    delay_ns(1000);
    uint16_t data_ADC = 0;
    for (int SPICount = 0; SPICount < 15; SPICount++) {
      mraa_gpio_write(pin_clk, 0);
      delay_ns(1000);
      mraa_gpio_write(pin_clk, 1);
      if (SPICount >= 3) {  // first 3 cycles doesn't matter
        data_ADC = (data_ADC << 1) + mraa_gpio_read(pin_data);
      }
      delay_ns(1000);
    }
    mraa_gpio_write(pin_cs, 1);  //SPI communication end
    float voltage = VREF * data_ADC / (1 << ADC_RESOLUTION);
    printf("%fv\n", voltage);
    delay_ms(10);
  }
  
  /* release resource section */
  mraa_gpio_close(pin_data);
  mraa_gpio_close(pin_cs);
  mraa_gpio_close(pin_clk);
}
