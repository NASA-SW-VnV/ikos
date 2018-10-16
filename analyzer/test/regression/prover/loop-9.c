extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main() {
  int i, s, n;

  n = __ikos_nondet_int();
  s = 0;
  for (i = 0; i < n; i++) {
    s++;
  }

  __ikos_assert(s == i);

  return 42;
}
