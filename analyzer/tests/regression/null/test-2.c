// DEFINITE UNSAFE
int* foo(int* x, int* y) {
  return x + *y;
}

int* p; // NULL

int main() {
  int a[10];
  int b = *p;

  int* z = foo(&a[0], &b);
  return *z;
}
