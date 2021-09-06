/**
 * P01.c
 * P46091204 Cheng-Ying Tsai Copyright (C) 2021
**/

#include <errno.h>
#include <mraa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <stdint.h>
#include "upboard_hat.h"
#include "util.h"

#define SERIAL_DEVICE "/dev/ttyS0"

static volatile sig_atomic_t stopped = 0;

void int_handler(int sig) {
  printf("Received signal %d\n", sig);
  stopped = 1;
  exit(sig);  
}

mraa_gpio_context pin_load, pin_din, pin_clk;

/**
 * Send a byte over the DIN pin
 */
void send_byte(uint8_t d) {
  for (int i = 7; i >= 0; --i) {
    mraa_gpio_write(pin_clk, 0);
    delay_ns(1000);
    mraa_gpio_write(pin_din, (d >> i) & 1u);
    delay_ns(1000);
    mraa_gpio_write(pin_clk, 1);
    delay_ns(1000);
  }
}

/**
 * Write data to the register at addr
 */
void write_reg(uint8_t addr, uint8_t data) {
  mraa_gpio_write(pin_clk, 1);
  mraa_gpio_write(pin_load, 0);
  send_byte(addr);
  send_byte(data);
  mraa_gpio_write(pin_clk, 0);
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


// Each bit corresponds to an LED in a row
// 00000001
const uint8_t dot_pattern = 0x18;
const uint8_t bar_pattern = 0xFF;

void showDot(){
  write_reg(4, dot_pattern);
  write_reg(5, dot_pattern);
  delay_ms(1000);
}

void showBar(){
  write_reg(4, bar_pattern);
  write_reg(5, bar_pattern);
  delay_ms(3000);
}

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

void toMorseCode(char* line, int length){  
  char* temp = NULL;
  char c;

  for (int i = 0; i < length-1; i++){
    c = line[i];

    if (c >= 32 && c <= 122){
      if (c==32) { temp = " "; }
      if (c==33) { temp = "-.-.--"; }
      if (c==34) { temp = ".-..-."; }
      if (c==36) { temp = "...-..-"; }
      if (c==38) { temp = ".-..."; }
      if (c==39) { temp = ".----."; }
      if (c==40) { temp = "-.--."; }
      if (c==41) { temp = "-.--.-"; }
      if (c==43) { temp = ".-.-."; }
      if (c==44) { temp = "--..--"; }
      if (c==45) { temp = "-....-"; }
      if (c==46) { temp = ".-.-.-"; }
      if (c==47) { temp = "-..-."; }
      if (c==48) { temp = "-----"; }      
      if (c==49) { temp = ".----"; }    
      if (c==50) { temp = "..---"; }     
      if (c==51) { temp = "...--"; }     
      if (c==52) { temp = "....-"; }       
      if (c==53) { temp = "....."; }    
      if (c==54) { temp = "-...."; }      
      if (c==55) { temp = "--..."; }      
      if (c==56) { temp = "---.."; }      
      if (c==57) { temp = "----."; }     
      if (c==58) { temp = "---..."; }
      if (c==59) { temp = "-.-.-."; }
      if (c==61) { temp = "-...-"; }
      if (c==63) { temp = "..--.."; }
      if (c==64) { temp = ".--.-."; }
      if (c==65 || c==97) { temp = ".-"; }
      if (c==66 || c==98) { temp = "-..."; }
      if (c==67 || c==99) { temp = "-.-."; }
      if (c==68 || c==100) { temp = "-.."; }
      if (c==69 || c==101) { temp = "."; }
      if (c==70 || c==102) { temp = "..-."; }
      if (c==71 || c==103) { temp = "--."; }
      if (c==72 || c==104) { temp = "...."; }
      if (c==73 || c==105) { temp = ".."; }
      if (c==74 || c==106) { temp = ".---"; }
      if (c==75 || c==107) { temp = "-.-"; }
      if (c==76 || c==108) { temp = ".--.."; }
      if (c==77 || c==109) { temp = "--"; }
      if (c==78 || c==110) { temp = "-."; }
      if (c==79 || c==111) { temp = "---"; }
      if (c==80 || c==112) { temp = ".--."; }
      if (c==81 || c==113) { temp = "--.-"; }
      if (c==82 || c==114) { temp = ".-."; }
      if (c==83 || c==115) { temp = "..."; }
      if (c==84 || c==116) { temp = "-"; }
      if (c==85 || c==117) { temp = "..-"; }
      if (c==86 || c==118) { temp = "...-"; }
      if (c==87 || c==119) { temp = ".--"; }
      if (c==88 || c==120) { temp = "-..-"; }
      if (c==89 || c==121) { temp = "-.--"; }
      if (c==90 || c==122) { temp = "--.."; }
      if (c==95) { temp = "..--.-"; }
      if (temp == NULL){
        printf("Something wrong..., there are wrong characters which can't be converted to Morse Codes.\n\r");
        break;
      }

      if (c != 32){
        for (size_t i = 0; i < strlen(temp); i++){
          if (temp[i] == '.') { printf("1"); showDot();}
          if (temp[i] == '-') { printf("111"); showBar();}
          write_reg(4, 0x00);
          write_reg(5, 0x00);
          if (i != strlen(temp)-1){
            printf("0");
            delay_ms(1000);
          }
        }
        temp = NULL;
      } else if (c == 32) {
        printf("0000000");
        delay_ms(7000);
        continue;
      }

      if (!(line[i+1] == 32 || i+1 == length-1)){
        printf("000");
        delay_ms(3000);
      }
    } else {
      printf("Something wrong..., there are wrong characters which can't be converted to Morse Codes.\n\r");
      break;
    }
  }
}  


int main() {

  signal(SIGINT, int_handler);

  pin_load = mraa_gpio_init(UP_HAT_MAX7219_LOAD);
  pin_din = mraa_gpio_init(UP_HAT_MAX7219_DIN);
  pin_clk = mraa_gpio_init(UP_HAT_MAX7219_CLK);

  if (!(pin_load && pin_din && pin_clk)) {
    fprintf(stderr, "Failed to initalize GPIO. Did you run with sudo?\n");
    return EXIT_FAILURE;
  }

  mraa_gpio_dir(pin_load, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_din, MRAA_GPIO_OUT);
  mraa_gpio_dir(pin_clk, MRAA_GPIO_OUT);

  init_matrix();

  for (int i = 1; i <= 8; ++i) {
    write_reg(i, 0x00);
  }

  int fd = open(SERIAL_DEVICE, O_RDWR | O_NOCTTY);
  if (fd == -1) {
    perror("open_port: Unable to open " SERIAL_DEVICE " - ");
  }
  FILE *file = fdopen(fd, "r+");
  if (!file) {
    perror("fdopen");
  }

  dprintf(fd, "\n\rServer will show what you input with corresponding Morse Codes. Let's test !\n\r");
  dprintf(fd, "Enter quit to quit\n\n\r");
  struct termios old_term;
  setup_terminal(fd, &old_term);
  char *line = NULL;
  int i = 0;

  while (1) {
    dprintf(fd, "\n\rInput> ");
    ssize_t length = getline(&line, &(size_t){0}, file);

    if (length == -1) {
      if (!errno) break;
      perror("getline");
      exit(EXIT_FAILURE);
    }

    if (length >= 1 && line[length - 1] == '\n') {
      line[length - 1] = '\0';
    }

    toMorseCode(line, length);

    if (!strcmp("quit", line)) break;
    printf("\n\rClient says: %s (length=%ld)\n\r", line, strlen(line));
    if (i == 0) break;
  }

  dprintf(fd, "\n\rBye !\n\r");
  free(line);
  restore_terminal(fd, &old_term);
  close(fd);

  /* release resource section */
  for (int i = 1; i <= 8; ++i) {
    write_reg(i, 0x00);
  }

  mraa_gpio_close(pin_load);
  mraa_gpio_close(pin_din);
  mraa_gpio_close(pin_clk);
}

