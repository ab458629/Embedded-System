/**
 * @file
 * Basic serial communication
 * 
 * Print every line received from the serial port
 * Might need to use a serial client (e.g., PuTTY)
 */

#include <errno.h>
#include <mraa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define SERIAL_DEVICE "/dev/ttyS0"

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

int main() {
  int fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);
  if (fd == -1) {
    perror("open_port: Unable to open " SERIAL_DEVICE " - ");
  }
  FILE *file = fdopen(fd, "r+");
  if (!file) {
    perror("fdopen");
  }

  dprintf(fd, "Please input something. Server will show the same string.\n");
  dprintf(fd, "Enter quit to quit\n\n");
  struct termios old_term;
  setup_terminal(fd, &old_term);
  char *line = NULL;
  while (1) {
    dprintf(fd, "input> ");
    ssize_t length = getline(&line, &(size_t){0}, file);
    if (length == -1) {
      if (!errno) break;
      perror("getline");
      exit(EXIT_FAILURE);
    }
    if (length >= 1 && line[length - 1] == '\n') {
      line[length - 1] = '\0';
    }
    if (!strcmp("quit", line)) break;
    printf("Client says: %s (length=%ld)\n", line, strlen(line));
  }
  dprintf(fd, "bye\n");
  free(line);
  restore_terminal(fd, &old_term);
  close(fd);
}
