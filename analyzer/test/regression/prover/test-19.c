extern int __ikos_nondet_int(void);
extern void __ikos_assert(int);

int main() {
  int i, j;
  j = 0;

  for (i = 0; i < 10; ++i) {
    if (__ikos_nondet_int()) {
      j++;
    }
    __ikos_assert(j >= 0 && j <= 10);
  }

  return 0;
}
