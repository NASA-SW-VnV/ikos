// DEFINITE UNSAFE

extern void __ikos_assert(int);

int x = 6;

void f4() {
  x++;
  x++;
}

void f3() {
  f4();
}

void f2() {
  f3();
}

void f1() {
  f2();
  __ikos_assert(x == 7); // x == 8
  f3();
  __ikos_assert(x == 8); // x== 10
  x++;
}

int main(int argc, char** argv) {
  f1();
  __ikos_assert(x == 11);
  return 42;
}
