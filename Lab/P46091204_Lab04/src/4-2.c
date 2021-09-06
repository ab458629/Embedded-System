/**
    4-2.c
    P46091204 Cheng-Ying Tsai
**/

#include <errno.h>
#include <mraa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define SERIAL_DEVICE "/dev/ttyS0"

#define NONE "\033[m"
#define RED "\033[0;32;31m"
#define LIGHT_RED "\033[1;31m"
#define GREEN "\033[0;32;32m"
#define LIGHT_GREEN "\033[1;32m"
#define BLUE "\033[0;32;34m"
#define LIGHT_BLUE "\033[1;34m"
#define DARY_GRAY "\033[1;30m"
#define CYAN "\033[0;36m"
#define LIGHT_CYAN "\033[1;36m"
#define PURPLE "\033[0;35m"
#define LIGHT_PURPLE "\033[1;35m"
#define BROWN "\033[0;33m"
#define YELLOW "\033[1;33m"
#define LIGHT_GRAY "\033[0;37m"
#define WHITE "\033[1;37m"

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
  struct termios old_term;
  setup_terminal(fd, &old_term);

  dprintf(fd, LIGHT_RED"Hello World! \n");
  dprintf(fd, LIGHT_GREEN"Hello World! \n");
  dprintf(fd, LIGHT_BLUE"Hello World! \n");
  dprintf(fd, DARY_GRAY"Hello World! \n");
  dprintf(fd, LIGHT_CYAN"Hello World! \n");
  dprintf(fd, LIGHT_PURPLE"Hello World! \n");
  dprintf(fd, BROWN"Hello World! \n");
  dprintf(fd, LIGHT_GRAY"Hello World! \n");
  dprintf(fd, WHITE"Hello World! \n");

  restore_terminal(fd, &old_term);
  close(fd);
}
