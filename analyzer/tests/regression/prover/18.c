extern void __ikos_assert(int);
extern int __ikos_unknown();
/*
 * Adapted from ex17.c in NECLA test suite
 */

int main() {
  int flag = __ikos_unknown();
  int a = __ikos_unknown();

  int b;
  int j = 0;

  for (b = 0; b < 100; ++b) {
    if (flag)
      j = j + 1;
  }

  if (flag) {
    __ikos_assert(j == 100);
  }
}
