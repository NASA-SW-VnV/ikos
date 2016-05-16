extern void __ikos_assert(int);
extern int __ikos_unknown();

#define TRUE 1

int main() {
  int n = 0;
  while (TRUE) {
    if (__ikos_unknown())
      continue;
    if (n < 60)
      n++;
    else
      n = 0;
    __ikos_assert(n >= 0 && n <= 60);
  }
  return 42;
}
