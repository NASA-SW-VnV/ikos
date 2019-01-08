#include <x86intrin.h>

extern void printv(__m128);

int main(int argc, char** argv) {
  __m128 m = _mm_set_ps(4, 3, 2, 1);
  m = _mm_shuffle_ps(m, m, 0x1B);
  printv(m);
}
