/**
    4-1.c
    P46091204 Cheng-Ying Tsai

    From table of sensor performance on page 3 in datasheet shows that for different resolutions corresponds to different measuring time.

    Measuring Time :
            Typical	   Max
    12 bits    14ms   16ms
    11 bits     7ms    8ms
    10 bits     4ms    5ms
    8 bits      2ms    3ms

    Typical values are recommended for calculating energy consumption while maximum values shall be applied for calculating waiting times in communication.
    Therefore, the answer of quesion 4.1 is maximum values, 16ms (for 12 bits), 8ms (for 11 bits), 5ms (for 10 bits), 3ms (for 8 bits).
**/

#include <signal.h>

#include "mraa.h"
#include "util.h"

#define I2C_BUS 0
#define I2C_ADDRESS 0x40
#define TEMPERATURE_MEASUREMENT 0xf3    // No Hold master mode
#define HUMIDITY_MEASUREMENT 0xf5       // No Hold master mode
#define READ_USER_REG 0xE7
#define WRITE_USER_REG 0xE6
#define SET_RESOLTION_11_BITS 0x83 // the resolution is set to 11 bits / 11 bits (for RH/Temp)
// discard the last 2 status bits: 1111_1100
#define DATA_MASK 0xfc                  // Bit 1 of the two LSBs indicates the  measurement type (‘0’: temperature, ‘1’: humidity )

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}

int main() {
  mraa_i2c_context i2c = mraa_i2c_init(I2C_BUS);
  MRAA_ASSERT(mraa_i2c_address(i2c, I2C_ADDRESS));

  signal(SIGINT, int_handler);

  // We can refer to page 13 in datasheet to change resolution of RH or Temp
  uint8_t buf_original[1];
  MRAA_ASSERT(mraa_i2c_write_byte(i2c, READ_USER_REG));
  mraa_i2c_read(i2c, buf_original, 1);  // User Register Content 0000 0010
  printf("Original User Register Content : %d\n\r", buf_original[0]);

  uint8_t buf_after[1];
  MRAA_ASSERT(mraa_i2c_write_byte_data(i2c, SET_RESOLTION_11_BITS, WRITE_USER_REG));  // set 11 bits / 11 bits for RH/Temp
  MRAA_ASSERT(mraa_i2c_write_byte(i2c, READ_USER_REG));
  mraa_i2c_read(i2c, buf_after, 1);  // User Register Content 1000 0011
  printf("After Changing User Register Content : %d\n\r", buf_after[0]);

  /*
  The measured data is transmitted in two bytes, with the Most Significant bit (MSb) sent first, and is leftaligned. 
  In other words, if the 11-bit relative humidity resolution is used, the first of the two bytes contains
  the eight MSbs of the 11-bit data, and bits seven, six, five of the second byte contain the remaining three
  bits of the 11-bit data. The remaining second byte is "don't care".
  */

  // FIXME: MRAA can report error if mraa_i2c_write_byte is interrupted

  while (!stopped) {

    uint8_t buf[2];
  
    MRAA_ASSERT(mraa_i2c_write_byte(i2c, TEMPERATURE_MEASUREMENT));
    delay_ms(50);
    mraa_i2c_read(i2c, buf, 2);
    uint16_t temp_raw = (buf[0] << 8) + (buf[1] & DATA_MASK);
    double temp = -46.85 + 175.72 * temp_raw / (1 << 16);

    MRAA_ASSERT(mraa_i2c_write_byte(i2c, HUMIDITY_MEASUREMENT));
    delay_ms(20);
    mraa_i2c_read(i2c, buf, 2);
    uint16_t hum_raw =
        (buf[0] << 8) + (buf[1] & DATA_MASK);  // discard the last 2 status bits
    double hum = -6.0 + 125.0 * hum_raw / (1 << 16);

    printf("TEMP: %.2f C\tHUM: %.2f %%\n", temp, hum);
    delay_ms(100);
  }
  mraa_i2c_stop(i2c);
}