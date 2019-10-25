#include <stddef.h>

extern void __ikos_abstract_mem(void* ptr, size_t size);

static int tab[10];

static int* f(int value) {
  for (int i = 0; i < 10; i++) {
    if (tab[i] == value) {
      return &tab[i];
    }
  }
  return NULL;
}

int main() {
  __ikos_abstract_mem(&tab[0], 10 * sizeof(int));
  int* p = f(42);
  if (p != NULL) {
    *p = 0;
  }
}
