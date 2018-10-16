extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * Adapted from ex17.c in NECLA test suite
 */

int main() {
  int flag = __ikos_nondet_int();
  int a = __ikos_nondet_int();

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
