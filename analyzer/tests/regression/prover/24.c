extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * "nested5.c" from InvGen test suite
 */

int main() {
  int i, j, k, n;

  n = __ikos_unknown();
  for (i = 0; i < n; i++)
    for (j = i; j < n; j++)
      for (k = j; k < n; k++)
        __ikos_assert(k >= i);
}
