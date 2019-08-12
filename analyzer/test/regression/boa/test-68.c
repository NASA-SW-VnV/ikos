void f1(void) {
  int tab[10];
  int* p = &tab[0];
  int* q = &tab[10];
  for (; p != q; p++) {
    *p = 0;
  }
}

void f2(void) {
  int tab[100];
  int* p = &tab[0];
  int* q = &tab[100];
  for (; p != q; p++) {
    *p = 0;
  }
}

int main() {
  f1();
  f2();
  return 0;
}
