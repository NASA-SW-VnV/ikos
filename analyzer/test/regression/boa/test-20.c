#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int __ikos_nondet_int(void);

float readFloatSerial() {
  uint8_t index = 0;
  uint8_t timeout = 0;
  char data[15] = "";

  do {
    if (__ikos_nondet_int() == 0) {
      timeout++;
    } else {
      data[index] = __ikos_nondet_int();
      timeout = 0;
      index++;
    }
  } while ((index == 0 || data[index - 1] != ';') && (timeout < 10) &&
           (index < sizeof(data) - 1));

  data[index] = '\0';

  return atof(data);
}

int main() {
  printf("%f", readFloatSerial());
  return 0;
}
