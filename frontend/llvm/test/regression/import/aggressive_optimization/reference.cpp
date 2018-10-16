void f(int& x) {
  x = 1;
}

int main() {
  int y = 0;
  f(y);
  return y;
}
