int a[100][100];
int b[2] = {1, 2};
int c;
int d = 5;
extern int e;

int main(int argc, char** argv) {
  int i = 0, j = 0;
  for (; i < 100; i++)
    for (; j < 100; j++)
      if (i % 2 == 0)
        a[i][j] = b[0] + c - e;
      else
        a[i][j] = b[1] + d - e;
  return 0;
}
