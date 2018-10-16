int g;

int foo(int x, int y) {
  int z = x - y;
  int a;
  if (g == 0 && z)
    a = x + y;
  else
    a = x * y;
  return a * 42;
}
