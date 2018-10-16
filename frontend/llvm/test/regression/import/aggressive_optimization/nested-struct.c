#include <stdlib.h>

struct Node {
  struct Node* next;
};

int main(int argc, char* argv[]) {
  struct Node* node = malloc(sizeof(struct Node*));
  return 0;
}
