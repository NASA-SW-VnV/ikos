#include <stdlib.h>

extern unsigned __ikos_nondet_uint(void);
extern void __ikos_assume_mem_size(const void* ptr, size_t size);

int main() {
  int* p = (int*)malloc(__ikos_nondet_uint());
  if (!p) {
    return -1;
  }
  *p = 1; // warning
  __ikos_assume_mem_size(p, sizeof(int));
  *p = 2; // safe
  __ikos_assume_mem_size(p, sizeof(char));
  *p = 3; // error
  return 0;
}
