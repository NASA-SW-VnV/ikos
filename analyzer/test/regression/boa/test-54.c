static int count = 10;

int main() {
  int tab[10] = {0};
  int i;
  for (i = 0; i < 10; i++) {
    count = count >> 1;
    tab[count] = 1;
  }
  return 0;
}
