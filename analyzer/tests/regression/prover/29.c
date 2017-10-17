extern void __ikos_assert(int);
extern int __ikos_unknown();

int main() {
  int a = 1;
  int b = 1;
  int c = 2;
  int d = 2;
  int x = 3;
  int y = 3;
  while (__ikos_unknown()) {
    x = a + c;
    y = b + d;
    if ((x + y) % 2 == 0) {
      a++;
      d++;
    } else {
      a--;
    }
    while (__ikos_unknown()) {
      c--;
      b--;
    }
  }
  __ikos_assert(a + c == b + d);
}
