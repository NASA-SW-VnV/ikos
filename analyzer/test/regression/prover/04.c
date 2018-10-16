extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

/*
 * Taken from Gulwani PLDI'08:
 * Program Analysis as Constraint Solving
 */

int main() {
  int x, y;

  x = -50;
  y = __ikos_nondet_int();
  while (x < 0) {
    x = x + y;
    y++;
  }
  __ikos_assert(y > 0);
}
