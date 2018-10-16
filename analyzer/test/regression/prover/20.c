extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int x = __ikos_nondet_int();
  int y = __ikos_nondet_int();
  int k = __ikos_nondet_int();
  int j = __ikos_nondet_int();
  int i = __ikos_nondet_int();
  int n = __ikos_nondet_int();

  if ((x + y) == k) { // assume((x+y)== k);
    int m = 0;
    j = 0;
    while (j < n) {
      if (j == i) {
        x++;
        y--;
      } else {
        y++;
        x--;
      }
      if (__ikos_nondet_int())
        m = j;
      j++;
    }
    __ikos_assert((x + y) == k);
    if (n > 0) {
      __ikos_assert(0 <= m);
      __ikos_assert(m < n);
    }
  }
}
