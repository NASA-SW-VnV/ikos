#include <complex.h>

int main() {
  double complex c = 1.0 + 2.0 * I;
  return creal(c);
}
