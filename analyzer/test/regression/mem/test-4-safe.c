// SAFE

extern void __ikos_assert(int);

struct bar {
  int y;
  unsigned char z;
};

struct foo {
  unsigned char a;
  long b;
  unsigned char c;
  struct bar d;
};

struct foo x = {5, 2000, 10, {32, 5}};

int h;

int main() {
  if (x.d.y > 0) {
    x.d.y++;
    h = 8;
    h++;
    __ikos_assert(x.d.y + x.c + h == 52);
  }
  return 42;
}
