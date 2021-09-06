#include <signal.h>

#include "mraa.h"
#include "util.h"

#define I2C_BUS 0
#define I2C_ADDRESS 0x40
#define TEMPERATURE_MEASUREMENT 0xf3
#define HUMIDITY_MEASUREMENT 0xf5
// discard the last 2 status bits: 1111_1100
#define DATA_MASK 0xfc

volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
}

int main() {
  mraa_i2c_context i2c = mraa_i2c_init(I2C_BUS);
  MRAA_ASSERT(mraa_i2c_address(i2c, I2C_ADDRESS));

  signal(SIGINT, int_handler);

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