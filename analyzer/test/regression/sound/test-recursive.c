extern void f1(void);
extern void f2(void);
extern void f3(void);

void f1(void) {
  f2();
}

void f2(void) {
  f3();
}

void f3(void) {
  f1();
}

int main() {
  f1();
  return 0;
}
