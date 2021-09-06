/**
 * @file
 * Basic serial communication and sensors
 * 
 * Receive commands over the serial port
 * Print sensor measurements back to the serial port
 * Command summary:
 *   'temp?': Temperature
 *   'hum?' : Relative humidity
 *   'quit' : End the program
 * Might need to use a serial client (e.g., PuTTY)
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "mraa.h"
#include "util.h"

#define SERIAL_DEVICE "/dev/ttyS0"
#define I2C_BUS 0

mraa_i2c_context i2c;

/**
 * Set up the TTY with file descriptor fd
 * Save the previous settings into old_term
 */
void setup_terminal(int fd, struct termios *old_term) {
  if (tcgetattr(fd, old_term) != 0) {
    perror("tcgetattr");
    exit(EXIT_FAILURE);
  }
  struct termios new_term = *old_term;
  new_term.c_lflag |= ECHO | ECHOE | ICANON | ICRNL;
  cfsetspeed(&new_term, B115200);  // set baud rate to 115200
  if (tcsetattr(fd, TCSANOW, &new_term) != 0) {
    perror("tcsetattr");
    exit(EXIT_FAILURE);
  }
}

/**
 * Restore the TTY with file descriptor fd with saved settings from old_term
 */
void restore_terminal(int fd, struct termios *old_term) {
  if (tcsetattr(fd, TCSANOW, old_term) != 0) {
    perror("tcsetattr");
    exit(EXIT_FAILURE);
  }
}

double read_temperature(mraa_i2c_context i2c) {
  uint8_t buf[2];

  mraa_i2c_write_byte(i2c, 0xf3);
  delay_ms(50);
  mraa_i2c_read(i2c, buf, 2);
  uint16_t temp_raw =
      (buf[0] << 8) + (buf[1] & 0xfc);  // discard the last 2 status bits
  return -46.85 + 175.72 * temp_raw / (1 << 16);
}

double read_humidity(mraa_i2c_context i2c) {
  uint8_t buf[2];

  mraa_i2c_write_byte(i2c, 0xf5);
  delay_ms(20);
  mraa_i2c_read(i2c, buf, 2);
  uint16_t hum_raw =
      (buf[0] << 8) + (buf[1] & 0xfc);  // discard the last 2 status bits
  return -6.0 + 125.0 * hum_raw / (1 << 16);
}

int main() {
  int fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);
  if (fd == -1) {
    perror("open_port: Unable to open " SERIAL_DEVICE " - ");
  }

  FILE *file = fdopen(fd, "r+");
  if (!file) {
    perror("fdopen");
  }

  i2c = mraa_i2c_init(I2C_BUS);
  if (!i2c) {
    fprintf(stderr, "Cannot initalize I2C bus %d\n", I2C_BUS);
    exit(EXIT_FAILURE);
  }
  mraa_i2c_address(i2c, 0x40);

  struct termios old_term;
  setup_terminal(fd, &old_term);
  char *line = NULL;
  dprintf(fd, "Enter 'temp?' for temperature, 'hum?' for humidity\r\n");
  dprintf(fd, "Enter quit to quit\n\n");
  for (;;) {
    dprintf(fd, "> ");
    ssize_t length = getline(&line, &(size_t){0}, file);
    if (length == -1) {
      if (!errno) break;
      // NOTE:
      // getline can return ENOINT if mraa_i2c_init is called before.
      // This shouldn't happen. MRAA bug?
      if (errno == ENOENT) break;
      perror("getline");
      exit(EXIT_FAILURE);
    }
    if (length >= 1 && line[length - 1] == '\n') line[length - 1] = '\0';
    if (!strcmp("temp?", line)) {
      dprintf(fd, "Temperature: %.2f C\r\n", read_temperature(i2c));
    } else if (!strcmp("hum?", line)) {
      dprintf(fd, "Humidity: %.2f %%\r\n", read_humidity(i2c));
    } else if (!strcmp("quit", line))
      break;
  }
  dprintf(fd, "bye\n");
  free(line);
  restore_terminal(fd, &old_term);
  close(fd);
}
