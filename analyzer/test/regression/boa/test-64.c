#include <string.h>
#include <unistd.h>

int main() {
  char buffer[32];
  read(0, buffer, 32);
  return strncmp(buffer, "hello", 32);
}
