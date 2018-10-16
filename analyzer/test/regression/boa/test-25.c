#include <stdint.h>

int main() {
  int accelSample[3];

  for (uint8_t axis = 0; axis < 3; axis++) {
    accelSample[axis] = 0;
  }

  return 0;
}
