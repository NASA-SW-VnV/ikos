extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * "nested5.c" from InvGen test suite
 */

int main() {
  int i, j, k, n;

  n = __ikos_nondet_int();
  for (i = 0; i < n; i++)
    for (j = i; j < n; j++)
      for (k = j; k < n; k++)
        __ikos_assert(k >= i);
}
