// DEFINITE UNSAFE

extern void __ikos_assert(int);

int foo(int* a, int* b, int* c) {
  int* p;
  int* q;

  p = a + 1;
  q = b + 2;

  if (p == q) {
    q = q - 10;
    p = p + 42;
  }
  __ikos_assert(*p == 3);
  __ikos_assert(*q == 6);

  int res = c[*p + *q];
  c[*p + *q] = 555;
  return res;
}

int main(int argc, char** argv) {
  int a[2];
  int b[3];
  int c[10];

  c[9] = 666;

  a[0] = 1;
  a[1] = 3;

  b[0] = 4;
  b[1] = 5;
  b[2] = 6;

  int x = foo(a, b, c);

  __ikos_assert(x == 666);
  __ikos_assert(c[9] == 666);
  return x;
}
