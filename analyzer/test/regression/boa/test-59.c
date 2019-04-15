#include <stddef.h>

extern void __ikos_check_mem_access(const void* ptr, size_t size);

int main() {
  char buffer[32];
  __ikos_check_mem_access(buffer, 32);
  return 0;
}
