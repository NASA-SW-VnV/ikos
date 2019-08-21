// SAFE
// This test does an aligned alloc with specific alignment properties and then
// does a realloc over it
//
// The over-alignment is not guaranteed anymore after a realloc have been done.
//
// WARNING: The non guaranteed alignment is the one specified by the
// aligned_alloc, not by the one guaranted by the realloc (not an UB)

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Only available since C11
extern void* aligned_alloc(size_t alignment, size_t size);

int main(int argc, char* argv[]) {
  int* aligned_ptr = aligned_alloc(1 << 12, sizeof(int));

  printf("Aligned pointer: %p, value %d\n", aligned_ptr, *aligned_ptr);

  int* possibly_non_aligned = realloc(aligned_ptr, 324000000 * sizeof(int));

  printf("unkown pointer:  %p, value %d",
         possibly_non_aligned,
         *possibly_non_aligned);

  return 0;
}
