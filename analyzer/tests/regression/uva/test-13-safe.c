int* foo(int* x, int* y) {
  int* z = x + *y;
  *z = 78;
  return x + *y;
}

int main() {
  int a[10];
  int b = 5;

  int* z = foo(&a[0], &b);
  return *z;
}
