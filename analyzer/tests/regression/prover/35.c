extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * InvGen, CAV'09 paper, fig 2
 */

int main(int argc, char** argv) {
  int n = __ikos_unknown();
  int x = 0;
  if (n < 0)
    return 0;
  while (x < n) {
    x++;
  }
  if (n > 0) {
    __ikos_assert(x == n);
  }
}
