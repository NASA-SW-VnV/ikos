extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * "nest-if8" from InvGen benchmark suite
 */

int main() {
  int i, j, k, n, m;
  n = __ikos_nondet_int();
  m = __ikos_nondet_int();
  if (m + 1 < n)
    ;
  else
    return 0;
  for (i = 0; i < n; i += 4) {
    for (j = i; j < m;) {
      if (__ikos_nondet_int()) {
        __ikos_assert(j >= 0);
        j++;
        k = 0;
        while (k < j) {
          k++;
        }
      } else {
        __ikos_assert(n + j + 5 > i);
        j += 2;
      }
    }
  }
  return 0;
}
