int main(int argc, char** argv) {
  int i = 0, a[10];
  bool flag = argc % 5 == 0;
  for (; i < 10; i++) {
    if (flag)
      a[i] = i ^ 2;
    else
      a[i] = i * 2;
  }
  return 0;
}
