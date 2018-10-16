// SAFE
extern void __ikos_assert(int);

typedef union _u {
  int a;
  char b[4];
} my_union;
my_union U;

typedef struct _x {
  unsigned int a : 1;
  unsigned int b : 1;
} bit;

int main() {
  bit z;
  z.b = 0;
  z.a = 1;
  __ikos_assert((z.a == 1));
  U.b[0] = 1;
  U.b[1] = 1;
  U.b[2] = 1;
  U.b[3] = 1;
  __ikos_assert((U.a == 0x1010101));
  return 42;
}
