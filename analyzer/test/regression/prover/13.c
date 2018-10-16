extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * Based on "Property-Directed Incremental Invariant Generation" by Bradley et
 * al.
 */

int main(int flag, char** argv) {
  int j = 2;
  int k = 0;

  while (__ikos_nondet_int()) {
    if (flag)
      j = j + 4;
    else {
      j = j + 2;
      k = k + 1;
    }
  }
  if (k != 0) {
    __ikos_assert(j == 2 * k + 2);
  }
}
