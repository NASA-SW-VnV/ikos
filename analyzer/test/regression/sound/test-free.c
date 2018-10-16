#include <stdlib.h>

extern char* os_name(void);

int main() {
  char* p = os_name();
  free(p);
  return 0;
}
