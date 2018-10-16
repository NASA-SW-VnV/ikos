extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * "split.c" from InvGen benchmark suite
 */

int main() {
  int k = 100;
  int b = __ikos_nondet_int();
  int j = __ikos_nondet_int();
  int n;
  int i = j;
  for (n = 0; n < 2 * k; n++) {
    if (b) {
      i++;
    } else {
      j++;
    }
    b = !b;
  }
  __ikos_assert(i == j);
}
