extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * From "The Octagon Abstract Domain" HOSC 2006 by Mine.
 */

int main() {
  int a = 0;
  int j;
  int m;
  m = __ikos_nondet_int();
  if (m <= 0)
    return 0;
  for (j = 1; j <= m; j++) {
    if (__ikos_nondet_int())
      a++;
    else
      a--;
  }
  __ikos_assert(a >= -m);
  __ikos_assert(a <= m);
}
