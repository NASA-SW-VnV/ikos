extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int a = 1;
  int b = 1;
  int c = 2;
  int d = 2;
  int x = 3;
  int y = 3;
  while (__ikos_nondet_int()) {
    x = a + c;
    y = b + d;
    if ((x + y) % 2 == 0) {
      a++;
      d++;
    } else {
      a--;
    }
    while (__ikos_nondet_int()) {
      c--;
      b--;
    }
  }
  __ikos_assert(a + c == b + d);
}
