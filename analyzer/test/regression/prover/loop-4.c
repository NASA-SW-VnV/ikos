extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int foo(int x, int y) {
  while (x < 100) {
    if (__ikos_nondet_int())
      y = 1;
    x = x + 4;
  }

  __ikos_assert(y >= 0 && y <= 1);
  __ikos_assert(x <= 103);

  return x + y;
}

int main() {
  int res = foo(0, 0);
  __ikos_assert(res <= 104);
}
