extern void __ikos_assert(int);
extern int __ikos_unknown();

/*
 * "fragtest_simple" from InvGen benchmark suite
 */

int main() {
  int i, pvlen;
  int t;
  int k = 0;
  int n;
  i = 0;

  pvlen = __ikos_unknown();
  //  pkt = pktq->tqh_first;
  while (__ikos_unknown())
    i = i + 1;
  if (i > pvlen) {
    pvlen = i;
  } else {
  }
  i = 0;

  while (__ikos_unknown()) {
    t = i;
    i = i + 1;
    k = k + 1;
  }
  while (__ikos_unknown())
    ;

  int j = 0;
  n = i;
  while (1) {
    __ikos_assert(k >= 0);
    k = k - 1;
    i = i - 1;
    j = j + 1;
    if (j < n) {
    } else {
      break;
    }
  }
  return 0;
}
