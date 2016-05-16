struct Foo {
  int x;
  double a[10];
};

void f(double* p, int n) {
  int i;
  for (i = 0; i < n; ++i) {
    p[i] = 1.0;
  }
}

int main() {
  struct Foo foo;
  f((double*)&foo.a, 10);
  return 0;
}
