// SAFE

int foo(const float* x, int xlen, float u) {
  int idx = 0;
  int bottom = 0;
  int top = xlen - 1;
  int retValue = 0;

  for (;;) {
    idx = (bottom + top) / 2;
    if (u <= x[idx])
      top = idx - 1;
    else if (u > x[idx + 1])
      bottom = idx + 1;
    else {
      retValue = idx;
      break;
    }
  }
  return retValue;
}

int main(int argc, char** argv) {
  int xlen = 100;
  float A[100];
  return foo(&A, xlen, 34.0);
}
