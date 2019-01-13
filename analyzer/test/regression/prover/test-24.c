extern void __ikos_assert(int);

struct {
  struct {
    int a;
  };
  double b[];
} c = {{9}};

int main() {
  __ikos_assert(c.a == 9);
  return 0;
}
