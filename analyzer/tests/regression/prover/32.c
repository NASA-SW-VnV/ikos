extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * "split.c" from InvGen benchmark suite
 */

int main() {
  int k = 100;
  int b;
  int i;
  int j;
  int n;
  i = j;
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
