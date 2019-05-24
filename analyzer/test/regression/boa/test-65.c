extern int __ikos_nondet_uint(void);

static unsigned char i;

int main() {
  int A[10];
  i = __ikos_nondet_uint();
  if (i < 10) {
    A[i] = 0;
  }
  return 0;
}
