#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

void delay_seconds(unsigned t) {
  sleep(t);
}

void delay_ns(long ns) {
  nanosleep(
      &(struct timespec){
          .tv_sec = ns / 1000000000,
          .tv_nsec = ns % 1000000000,
      },
      NULL);
}

void delay_ms(double ms) { delay_ns(ms * 1000000); }

// Check MRAA return status
#define MRAA_ASSERT(ret)                  \
  do {                                    \
    mraa_result_t res = (ret);            \
    if (res != MRAA_SUCCESS) {            \
      mraa_result_print(res);             \
      printf("Did you run with sudo?\n"); \
      exit(EXIT_FAILURE);                 \
    }                                     \
  } while (0)
