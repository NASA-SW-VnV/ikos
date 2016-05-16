extern void __ikos_assert(int);
extern int __ikos_unknown();

int main() {
  int i, s, n;

  n = __ikos_unknown();
  s = 0;
  for (i = 0; i < n; i++) {
    s++;
  }

  __ikos_assert(s == i);

  return 42;
}
