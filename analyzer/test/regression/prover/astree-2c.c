extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

// Need of the clock domain or fully relational
int main() {
  int i = 1000;
  int x = 1000;
  int b = __ikos_nondet_int();
  while (i >= -200) {
    x--;
    if (b)
      x = 1000;
    i--;
  }

  __ikos_assert(x <= 1000);
  return 42;
}
