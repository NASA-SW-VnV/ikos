#include <stddef.h>

extern void __ikos_assert(int);
extern unsigned __ikos_nondet_uint(void);
extern void __ikos_forget_mem(void* ptr, size_t size);
extern void __ikos_abstract_mem(void* ptr, size_t size);

typedef struct {
  int integers[2];
  int* pointers[2];
} Struct;

int main() {
  int x = 3, y = 3;
  Struct s = {{1, 2}, {&x, &y}};

  __ikos_assert(s.integers[0] == 1);                         // OK
  __ikos_assert(s.integers[1] == 2);                         // OK
  __ikos_assert(*s.pointers[__ikos_nondet_uint() % 2] == 3); // OK

  __ikos_abstract_mem(&s.integers[0], sizeof(s.integers));
  __ikos_assert(s.integers[0] == 1);                         // NO
  __ikos_assert(s.integers[1] == 2);                         // NO
  __ikos_assert(*s.pointers[__ikos_nondet_uint() % 2] == 3); // OK

  __ikos_forget_mem(&s.integers[0], sizeof(s.integers));
  __ikos_assert(s.integers[0] == 1);                         // NO
  __ikos_assert(s.integers[1] == 2);                         // NO
  __ikos_assert(*s.pointers[__ikos_nondet_uint() % 2] == 3); // NO
}
