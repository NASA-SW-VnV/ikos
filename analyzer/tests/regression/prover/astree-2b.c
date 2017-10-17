extern void __ikos_assert(int);
extern int __ikos_unknown();

// Need of the clock domain or fully relational
int main() {
  int i = 5;
  int x = 5;
  int b = __ikos_unknown();
  while (i < 1000) {
    x = x + 2;
    if (b)
      x = 0;
    i = i + 2;
  }

  __ikos_assert(x <= 1000);
  return 42;
}
