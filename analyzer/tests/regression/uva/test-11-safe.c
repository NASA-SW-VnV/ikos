struct Foo {
  int x;
  double a[10];
};

void f(struct Foo* p) {
  int i;
  for (i = 0; i < 10; ++i) {
    p->a[i] = 1.0;
  }
}

int main() {
  struct Foo foo;
  f(&foo);
  return 0;
}
