// SAFE
struct s {
  struct s* next;
  int data;
};
struct s A[100];
int main() {
  int i = 0;
  struct s* ptr;
  for (i = 0; i < 199; i++) {
    if (i < 99)
      A[i].next = &(A[i + 1]);
    else
      A[i - 99].data = i;
  }
  A[99].next = 0;
  A[99].data = 99;
  ptr = &(A[0]);
  while (ptr != 0) {
    ptr = ptr->next;
  }
  return 42;
}
