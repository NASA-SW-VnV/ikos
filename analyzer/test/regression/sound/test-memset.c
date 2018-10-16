#include <string.h>

extern char* os_name(void);

int main() {
  char* p = os_name();
  memset(p, 0, 7);
  return 0;
}
