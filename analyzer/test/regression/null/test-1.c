// SAFE
int* foo(int* x, int* y) {
  return x + *y;
}

int main() {
  int a[10] = {0};
  int b = 5;

  int* z = foo(&a[0], &b);
  return *z;
}
