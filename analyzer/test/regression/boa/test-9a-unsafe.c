// DEFINITE UNSAFE
//
// If the multidimensional array is global then LLVM generates a
// single GetElementPtr instruction

int p[2][2][2];
int kalman_global(void) {
  p[0][0][0] = 1;
  p[0][0][1] = 1;
  p[0][1][0] = 1;
  p[0][1][1] = 1;
  p[2][0][0] = 1;
  p[1][0][1] = 1;
  p[1][1][0] = 1;
  p[1][1][1] = 1;

  return p[0][1][0] + p[0][1][1];
}

int main(int arg, char** argv) {
  return kalman_global();
}
