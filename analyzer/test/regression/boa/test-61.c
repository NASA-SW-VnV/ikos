#include <string.h>

extern void __ikos_check_string_access(const char* str);
extern void __ikos_abstract_mem(void* ptr, size_t size);

int main() {
  __ikos_check_string_access("hello world"); // safe

  char buffer[4] = {0};
  __ikos_check_string_access(buffer); // safe

  __ikos_abstract_mem(buffer, sizeof(buffer));
  __ikos_check_string_access(buffer); // unsafe, but analyzer says safe

  strcpy(buffer, "AAA");
  __ikos_check_string_access(buffer); // safe

  buffer[3] = 'A';
  __ikos_check_string_access(buffer); // unsafe, but analyzer says safe

  __ikos_check_string_access(&buffer[4]); // unsafe

  return 0;
}
