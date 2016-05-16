extern void __ikos_assert(int);
extern int __ikos_unknown();

int main() {
  int x = __ikos_unknown();
  int y = __ikos_unknown();
  int k = __ikos_unknown();
  int j = __ikos_unknown();
  int i = __ikos_unknown();
  int n = __ikos_unknown();

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
      if (__ikos_unknown())
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
