// SAFE

int foo(const float* x, int xlen, float u) {
  int idx = 0;
  int bottom = 0;
  int top = xlen - 1;
  int retValue = 0;
  unsigned int returnStatus = 0U;

  /*
   * Deal with the extreme cases first:
   *   if u <= x[bottom] then return idx = bottom
   *   if u >= x[top]    then return idx = top-1
   */
  if (u <= x[bottom]) {
    retValue = bottom;
    returnStatus = 1U;
  } else if (u >= x[top]) {
    retValue = top - 1;
    returnStatus = 1U;
  } else {
    /* else required to ensure safe programming, even *
     * if it's expected that it will never be reached */
  }

  if (returnStatus == 0U) {
    if (u < 0) {
      /* For negative input find index such that: x[idx] <= u < x[idx+1] */
      for (;;) {
        idx = (bottom + top) / 2;

        if (u < x[idx]) {
          top = idx - 1;
        } else if (u >= x[idx + 1]) {
          bottom = idx + 1;
        } else {
          /* we have x[idx] <= u < x[idx+1], return idx */
          retValue = idx;
          break;
        }
      }
    } else {
      /* For non-negative input find index such that: x[idx] < u <= x[idx+1] */
      for (;;) {
        idx = (bottom + top) / 2;

        if (u <= x[idx]) {
          top = idx - 1;
        } else if (u > x[idx + 1]) {
          bottom = idx + 1;
        } else {
          /* we have x[idx] < u <= x[idx+1], return idx */
          retValue = idx;
          break;
        }
      }
    }
  }

  return retValue;
}

int main(int argc, char** argv) {
  int xlen = 100;
  float A[100];
  float elem = 34.0;
  return foo(&A, xlen, elem);
}
