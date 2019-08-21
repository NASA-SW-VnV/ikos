// SAFE

int foo(int* a, int* b, int* c) {
  int* p;
  int* q;

  p = a + 1;
  q = b + 2;

  if (p == q) {
    q = q - 10;
    p = p + 42;
  }

  return c[*p + *q];
}

int main(int argc, char** argv) {
  int a[2];
  int b[3];
  int c[10] = {0};

  a[0] = 1;
  a[1] = 3;

  b[0] = 4;
  b[1] = 5;
  b[2] = 6;

  return foo(a, b, c);
}
