// From http://blog.regehr.org/archives/519
int foo1(void) {
  int y, z;
  for (y = 0; y < 5; y++)
    z++;
  return z; // undefined
}

int foo2(void) {
  int a[15];
  return a[10]; // undefined
}

void bar(int* p) {}

int foo3(void) {
  int x;
  bar(&x);
  return x; // undefined
}

int main() {
  int x = foo1();
  int y = foo2();
  int z = foo3();
  return 42;
}
