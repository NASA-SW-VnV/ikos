extern void __ikos_assert(int);
extern int __ikos_unknown();

int main(int flag, char** argv) {
  int i, j, k;
  j = 1;
  if (flag) {
    i = 0;
  } else {
    i = 1;
  }

  while (__ikos_unknown()) {
    i += 2;
    if (i % 2 == 0) {
      j += 2;
    } else
      j++;
  }

  int a = 0;
  int b = 0;

  while (__ikos_unknown()) {
    a++;
    b += (j - i);
  }
  if (flag) {
    __ikos_assert(a == b);
  }
}
