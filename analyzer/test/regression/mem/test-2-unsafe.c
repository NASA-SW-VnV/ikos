// DEFINITE UNSAFE

int main(int argc, char** argv) {
  int a[2];
  int b[3];
  int c[10] = {0};

  a[0] = 1;
  a[1] = 3;

  b[0] = 4;
  b[1] = 7;
  b[2] = 7;

  // b[1]=5;

  int* p = &a[0];
  int* q = &b[0];

  p = a + 1;
  q = b + 2;

  if (p == q) {
    q = q - 10;
    p = p + 42;
  }

  return c[*p + *(q - 1)];
}
