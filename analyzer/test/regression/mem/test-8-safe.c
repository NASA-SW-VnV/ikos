// SAFE

extern void __ikos_assert(int);

int foo(int* a, int* c) {
  int* p;

  p = a + 1;

  __ikos_assert(*p == 3);

  int res = c[*p];
  c[*p] = 555;
  return res;
}

int main(int argc, char** argv) {
  int a[2];
  int c[10];

  a[1] = 3;
  c[3] = 666;

  int x = foo(a, c);

  __ikos_assert(x == 666);
  __ikos_assert(c[3] == 555);
  return x;
}
