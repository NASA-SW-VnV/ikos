#include <stdint.h>
#include <string.h>

extern ssize_t read(int fd, void* buffer, size_t nbytes);

int main() {
  char buffer[32];
  read(0, buffer, 32);
  return strncmp(buffer, "hello", 32);
}
