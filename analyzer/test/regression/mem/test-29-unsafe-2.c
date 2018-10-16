// DEFINITE UNSAFE
extern void __ikos_assert(int);

struct point {
  int x;
  int y;
  int z;
};

int main(int argc, char** argv) {
  struct point p;
  p.x = 1;
  p.y = 2;
  p.z = 8;

  int* q = (int*)(((char*)&p.x) + 2);
  *q = 3;

  __ikos_assert(p.y == 2); // WARNING

  return 0;
}
