/**
    4-3.c
    P46091204 Cheng-Ying Tsai

    Terminal : The terminal is a GUI based program that runs under the GUI in Linux that EMULATES a tty

    tty : A true tty emulates an old style teletype machine, like the kind that was used in the 1960's and before.
          There is NO GUI in a tty, no mouse and no scrolling other then command line.
      
    Shell : A shell is a command line interpreter that reads and executes commands that are given to it from either -
            the terminal or tty. It works with both, but is neither. 

**/

#include <errno.h>
#include <mraa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>

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

  new_term.c_lflag |= ECHO | ECHOE | ICRNL | NOFLSH;
  new_term.c_lflag &= ~(ICANON | ECHOCTL);

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

#define max(a,b)(a>b?a:b)
#define min(a,b)(a<b?a:b)


int my_getline(char* line, int maxSize, FILE* fp){  
    char c;  
    int len = 0;  
    int index = 0;

    char line_[maxSize];

    do {
      c = fgetc(fp);

      while (c == 27) { // if the first value is esc
        c = fgetc(fp); // skip the [
        switch(c = fgetc(fp)) { // the real value
            case 67:  // for arrow right
                index = min(len, index + 1);
                break;
            case 68:  // for arrow left
                index = max(0, index - 1);
                break;
        }
        c = fgetc(fp);
      }

      if (c >= 32 && c <= 126) {
        for (int i = 0; i < len; i++){line_[i] = line[i];}  // copy line to line_

        len++;

        for (int i = 0; i < index; i++){line[i] = line_[i];}  
        line[index] = c;                                    // insert the character to line
        for (int i = index+1; i < len; i++){line[i] = line_[i-1];}

        index++;
      }

      if ('\n' == c)  break;

    } while( c != EOF && len < maxSize );

    line[len] = '\0';  
    return len;  
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

  dprintf(fd, "\n\rPlease input something. Server will show the same string.\n\r");
  dprintf(fd, "Enter quit to quit\n\n\r");
  struct termios old_term;
  setup_terminal(fd, &old_term);
  int maxSize = 20;
  char *line = (char*)malloc( sizeof(char) * maxSize);
  int i = 0;
  while (1) {
    dprintf(fd, "\n\rinput> ");
    // ssize_t length = readline(&line, &(size_t){0}, file);
    int length = my_getline(line, maxSize, file);


    if (length == -1) {
      if (!errno) break;
      perror("getline");
      exit(EXIT_FAILURE);
    }

    if (length >= 1 && line[length - 1] == '\n') {
      line[length - 1] = '\0';
    }

    if (!strcmp("quit", line)) break;
    printf("\n\rClient says: %s (length=%ld)\n\r", line, strlen(line));
    if (i == 0) break;
  }
  dprintf(fd, "\n\rbye\n\r");
  free(line);
  restore_terminal(fd, &old_term);
  close(fd);
}
