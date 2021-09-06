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

mraa_gpio_context pin_clk_ADC, pin_data, pin_cs;

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}

mraa_gpio_context pin_clk_MATRIX, pin_load, pin_din;

/**
 * Send a byte over the DIN pin
 */
void send_byte(uint8_t d) {
  for (int i = 7; i >= 0; --i) {
    mraa_gpio_write(pin_clk_MATRIX, 0);
    delay_ns(1000);
    mraa_gpio_write(pin_din, (d >> i) & 1u);
    delay_ns(1000);
    mraa_gpio_write(pin_clk_MATRIX, 1);
    delay_ns(1000);
  }
}

/**
 * Write data to the register at addr
 */
void write_reg(uint8_t addr, uint8_t data) {
  mraa_gpio_write(pin_clk_MATRIX, 1);
  mraa_gpio_write(pin_load, 0);
  send_byte(addr);
  send_byte(data);
  mraa_gpio_write(pin_clk_MATRIX, 0);
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
  // This line is used for resource collection. Ignore it.
  signal(SIGINT, int_handler);

  // pin 12(CLK): clock pin
  pin_clk_ADC = mraa_gpio_init(UP_HAT_MCP3201_CLK);
  // pin 38(DOUT): SPI data pin 
  pin_data = mraa_gpio_init(UP_HAT_MCP3201_DOUT);
  // pin 40(CS/SHDN): pin_cs used to initiate communication with the device
  pin_cs = mraa_gpio_init(UP_HAT_MCP3201_CS);

  pin_load = mraa_gpio_init(UP_HAT_MAX7219_LOAD);
  pin_din = mraa_gpio_init(UP_HAT_MAX7219_DIN);
  pin_clk_MATRIX = mraa_gpio_init(UP_HAT_MAX7219_CLK);

  mraa_gpio_dir(pin_clk_ADC, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_data, MRAA_GPIO_IN);
  mraa_gpio_dir(pin_cs, MRAA_GPIO_OUT);

  mraa_gpio_dir(pin_load, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_din, MRAA_GPIO_OUT);

  uint8_t row_pattern = 0xFF;

  while (!stopped) {
    mraa_gpio_write(pin_cs, 0);  // SPI communication start
    delay_ns(1000);
    uint16_t data_ADC = 0;
    for (int SPICount = 0; SPICount < 15; SPICount++) {
      mraa_gpio_write(pin_clk_ADC, 0);
      delay_ns(1000);
      mraa_gpio_write(pin_clk_ADC, 1);
      if (SPICount >= 3) {  // first 3 cycles doesn't matter
        data_ADC = (data_ADC << 1) + mraa_gpio_read(pin_data);
      }
      delay_ns(1000);
    }
    mraa_gpio_write(pin_cs, 1);  //SPI communication end
    float voltage = VREF * data_ADC / (1 << ADC_RESOLUTION);
    printf("%fv\n", voltage);
    
    float percentage = voltage / 3.3;
    printf("Percentage = %f\n", percentage);
 
    if (percentage < 0.111){
        row_pattern = 0xFF;
    } 
    else if (percentage >= 0.888){
        row_pattern = 0x00;
    }
    else{
        if (percentage >= 0.111 && percentage < 0.222){
            row_pattern = 0xFF>>1;
        }
        else if (percentage >= 0.222 && percentage < 0.333){
            row_pattern = 0xFF>>2;
        }
        else if (percentage >= 0.333 && percentage < 0.444){
            row_pattern = 0xFF>>3;
        }
        else if (percentage >= 0.444 && percentage < 0.555){
            row_pattern = 0xFF>>4;
        }
        else if (percentage >= 0.555 && percentage < 0.666){
            row_pattern = 0xFF>>5;
        }
        else if (percentage >= 0.666 && percentage < 0.777){
            row_pattern = 0xFF>>6;
        }
        else if (percentage >= 0.777 && percentage < 0.888){
            row_pattern = 0xFF>>7;
        }
    }
    // printf("row_pattern = %d\n", row_pattern);
    for (int i = 1; i <= 8; ++i) {
       write_reg(i, row_pattern);
    }

    delay_ms(100);
  }
  
  /* release resource section */
  mraa_gpio_close(pin_data);
  mraa_gpio_close(pin_cs);
  mraa_gpio_close(pin_clk_ADC);

  for (int i = 1; i <= 8; ++i) {
    write_reg(i, 0x00);
  }
  mraa_gpio_close(pin_load);
  mraa_gpio_close(pin_din);
  mraa_gpio_close(pin_clk_MATRIX);
}
