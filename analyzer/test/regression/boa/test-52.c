// DEFINITE UNSAFE

void foo(int** x) {
  x[3][2] = 5;
}

int main(int arg, char** argv) {
  int b[4][4];
  foo(b);
  return 0;
}
