extern int __ikos_nondet_int(void);

int main() {
  int x = __ikos_nondet_int();
  if (x < 0) {
    return -x;
  } else {
    return 0;
  }
}
