#include <stdio.h>

extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

// void(int)
static void a(int x) {}

// void(long)
static void b(long x) {}

// void(double)
static void c(double x) {}

// void(int*)
typedef void (*fun_ptr_t)(int*);

int main() {
  void* vtable[3];
  vtable[0] = (void*)a;
  vtable[1] = (void*)b;
  vtable[2] = (void*)c;

  int nd = __ikos_nondet_int();
  if (nd >= 0 && nd <= 2) {
    fun_ptr_t f = (fun_ptr_t)vtable[nd];
    f(NULL); // error
  }

  return 0;
}
