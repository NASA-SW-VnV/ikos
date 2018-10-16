extern void __ikos_assert(int);

// provable by using a fully relational or gauge domain but as well
// by using simply intervals after induction variable optimization.
int main() {
  int i, s, n;

  n = 100;
  s = 0;
  for (i = 0; i < n; i++) {
    s++;
  }

  __ikos_assert(s == i);

  return 42;
}
