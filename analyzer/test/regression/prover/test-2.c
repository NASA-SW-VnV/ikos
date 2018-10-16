extern void __ikos_assert(int);

int x = 4;
int y = 5;
float z = 8;
int main() {
  while (x < 10) {
    x++;
  }
  y--;
  z = z + y;

  __ikos_assert(x + y + z == 26);
  return 0;
}
