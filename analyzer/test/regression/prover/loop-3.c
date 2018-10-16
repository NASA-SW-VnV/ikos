extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int x = 0;
  int y = 0;
  while (x < 100) {
    if (__ikos_nondet_int())
      y = 1;
    x = x + 4;
  }

  __ikos_assert(y >= 0 && y <= 1);
  __ikos_assert(x <= 103);

  return 42;
}
