extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * IC3 motivating example
 */

int main() {
  int x = 1;
  int y = 1;

  while (__ikos_unknown()) {
    int t1 = x;
    int t2 = y;
    x = t1 + t2;
    y = t1 + t2;
  }

  __ikos_assert(y >= 1);
}
