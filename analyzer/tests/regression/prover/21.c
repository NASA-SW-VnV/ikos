extern void __ikos_assert(int);
extern int __ikos_unknown();
/*
 * Based on "larg_const.c" from InvGen test suite
 */

int main(int argc, char** argv) {
  int c1 = 4000;
  int c2 = 2000;
  int n, v;
  int i, k, j;

  n = __ikos_unknown();
  if (n > 0 && n < 10) {
    k = 0;
    i = 0;
    while (i < n) {
      i++;
      if (__ikos_unknown() % 2 == 0)
        v = 0;
      else
        v = 1;
      if (v == 0)
        k += c1;
      else
        k += c2;
    }

    __ikos_assert(k > n);
  }
  return 0;
}
