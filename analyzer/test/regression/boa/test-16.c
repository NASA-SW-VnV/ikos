// SAFE
struct foo {
  int x;
  int y;
  int z;
};

void init(struct foo* f) {
  f->x = 5;
  f->y = 6;
  f->z = 7;
}

int main(int argc, char** argv) {
  struct foo a;
  init(&a);
  return a.x;
}
