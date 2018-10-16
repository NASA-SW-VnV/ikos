void f(int* p) {
  *p = 0x42;
}

int main() {
  int* p;
  f(p);
  return 0;
}
