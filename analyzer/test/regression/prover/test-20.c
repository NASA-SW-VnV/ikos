extern void __ikos_assert(int);

int f() {
  return 1;
}
unsigned g() {
  return 1;
}

int main() {
  int* p = (int*)f();
  int* q = (int*)g();
  int* r = (int*)1;
  __ikos_assert(p == q);
  __ikos_assert(p == r);
  return 0;
}
