#include <stdio.h>

extern int __ikos_unknown();
extern void __ikos_assert(int);

int g = 0;

// void(int)
void a(int x) {
  g = 1;
}

// void(double)
void b(double x) {
  g = 2;
}

// void(int*)
void c(int* x) {
  g = 3;
}

// void(int*)
typedef void (*fun_ptr_t)(int*);

int main() {
  void* vtable[3];
  vtable[0] = (void*)a;
  vtable[1] = (void*)b;
  vtable[2] = (void*)c;

  int nd = __ikos_unknown();
  if (nd >= 0 && nd <= 2) {
    fun_ptr_t f = (fun_ptr_t)vtable[nd];
    f(NULL);
    __ikos_assert(g == 3);
  }

  return 0;
}
