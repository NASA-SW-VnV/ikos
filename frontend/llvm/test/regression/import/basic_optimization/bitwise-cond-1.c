int foo(int x, int y, int z) {
  int a = x - y;
  int b = (z == 0 && a) ? x + y : y + z;
  return (a > b) ? x : y;
}
