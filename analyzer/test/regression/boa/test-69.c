#include <stdio.h>
#include <stdlib.h>

static FILE* log = NULL;
static int* array = NULL;

int init() {
  // Open a file
  FILE* f = fopen("log", "w");
  if (f == NULL) {
    return EXIT_FAILURE;
  }
  log = f;

  // Allocate the array
  void* p = malloc(100 * sizeof(100));
  if (p == NULL) {
    return EXIT_FAILURE;
  }
  array = (int*)p;

  return EXIT_SUCCESS;
}

int main() {
  int status = init();
  if (status != EXIT_SUCCESS) {
    return status;
  }

  array[0] = 0;
  return EXIT_SUCCESS;
}
