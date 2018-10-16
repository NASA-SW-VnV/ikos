struct info_t {
  char x : 1;
  char y : 1;
  int z : 10;
  int k : 1;
};

int main() {
  struct info_t info = {0, 0, 42, 1};
  return 0;
}
