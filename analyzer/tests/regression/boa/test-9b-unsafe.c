// DEFINITE UNSAFE
//
// If the multidimensional array is local then LLVM generates multiple
// GetElementPtr instructions
int kalman_local(void) {
  int p[2][2][2];

  p[0][0][2] = 1;
  p[0][0][1] = 1;
  p[0][1][0] = 1;
  p[0][1][1] = 1;
  p[1][0][0] = 1;
  p[2][0][1] = 1;
  p[1][1][0] = 1;
  p[1][1][1] = 1;

  return p[0][1][0] + p[0][1][1];
}

int main(int arg, char** argv) {
  return kalman_local();
}
