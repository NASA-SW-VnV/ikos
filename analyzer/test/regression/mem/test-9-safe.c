// SAFE

extern void __ikos_assert(int);

int x = 6;
int y = 4;

void f4() {
  x++;
  y++;
}

void f3() {
  f4();
}

void f2() {
  f3();
}

void f1() {
  f2();
  __ikos_assert(x == 7 && y == 5);
  f3();
  __ikos_assert(x == 8 && y == 6);
  x++;
}

int main(int argc, char** argv) {
  f1();
  __ikos_assert(x == 9 && y == 6);
  return 42;
}
