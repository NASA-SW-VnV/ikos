// From http://blog.regehr.org/archives/519

int main(void) {
  int a[6], i;
  for (i = 0; i < 6; i++) {
    a[i] = 1;
  }
  return a[5];
}
