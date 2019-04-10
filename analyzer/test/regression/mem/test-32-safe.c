#include <string.h>

extern void __ikos_assert(int);

typedef struct {
  char message[255];
  int id;
} Struct;

int main() {
  Struct s = {.message = "", .id = 1};
  strncpy(&s.message[0], "hello world", 255);
  __ikos_assert(s.id == 1);
  return 0;
}
