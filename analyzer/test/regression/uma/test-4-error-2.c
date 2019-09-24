void bar(int* p) {}

int main(void) {
  int x;
  bar(&x);
  return x; // undefined
}
