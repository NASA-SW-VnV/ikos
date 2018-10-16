extern void __ikos_assert(int);

// need of relational domain

int main() {
  int X = 100000, Y = 1000000;
  while (X >= 0) {
    X--;
    Y--;
  }
  __ikos_assert(X <= Y);
  return 42;
}
