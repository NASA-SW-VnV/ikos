extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int main(int argc, char** argv) {
  int n = __ikos_nondet_int();
  int k = 1;
  int i = 1;
  int j = 0;
  if (n < 0)
    return 0;
  while (i < n) {
    j = 0;
    while (j < i) {
      k += (i - j);
      j++;
    }
    i++;
  }
  __ikos_assert(k >= n);
}
