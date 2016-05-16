extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * InvGen, CAV'09 paper, fig 2
 */

int main(int n, char** argv) {
  int x = 0;
  while (x < n) {
    x++;
  }
  if (n > 0) {
    __ikos_assert(x == n);
  }
}
