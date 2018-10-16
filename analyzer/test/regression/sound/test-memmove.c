#include <string.h>

extern char* os_name(void);

int main() {
  char* p = os_name();
  memmove(p, "Linux", 6);
  return 0;
}
