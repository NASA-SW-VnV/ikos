extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

// Need of the clock domain or fully relational
int main() {
  int i = 0;
  int x = 0;
  int b = __ikos_nondet_int();
  while (i < 1000) {
    x++;
    if (b)
      x = 0;
    i++;
  }

  __ikos_assert(x <= 1000);
  return 42;
}
