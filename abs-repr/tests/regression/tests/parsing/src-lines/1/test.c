double a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

int main(int argc, char** argv) {
  int i = 0;
  for (; i < 10; i++) {
    a[i] = a[i] * 0.88;
  }
  a[i] = i;
}
