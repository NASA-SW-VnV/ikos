
extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * From "A Practical and Complete Approach to Predicate Refinement" by McMillan
 * TACAS'06
 */

int main(int argc, char** argv) {
  int i = __ikos_unknown();
  int j = __ikos_unknown();

  int x = i;
  int y = j;

  while (x != 0) {
    x--;
    y--;
  }
  if (i == j) {
    __ikos_assert(y == 0);
  }
}
