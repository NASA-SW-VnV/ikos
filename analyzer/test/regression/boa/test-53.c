static int* p;

int main() {
  int A[10];
  p = &A[0];
  for (int i = 0; i < 10; i++) {
    *p = 0;
    p++;
  }
  return 0;
}
