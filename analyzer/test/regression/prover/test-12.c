extern void __ikos_assert(int);

void f(int* p, int* q) {
  if (p == q) {
    __ikos_assert(0);
  } else {
    __ikos_assert(1);
  }
}

int main() {
  int a[10];
  int b[10];
  f(&a[0], &b[0]);

  return 0;
}
