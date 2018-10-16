extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * From CAV'12 by Sharma et al.
 */

int main() {
  int x = 0;
  int y = 0;
  int n = 0;
  while (__ikos_nondet_int()) {
    x++;
    y++;
  }
  while (x != n) {
    x--;
    y--;
  }
  __ikos_assert(y == n);
}
