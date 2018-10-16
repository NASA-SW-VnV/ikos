extern void __ikos_assert(int);
extern int __ikos_nondet_int(void);

int x = 0;

void f(void) {
  x++;
  __ikos_assert(x == 1);
}

void g(void) {
  f();
  __ikos_assert(x == 1);
}

typedef void (*fun_ptr_t)(void);

int main(int argc, char** argv) {
  fun_ptr_t tab[2];
  tab[0] = &f;
  tab[1] = &g;

  int i = __ikos_nondet_int();
  if (i < 0 || i > 1) {
    return 0;
  }

  fun_ptr_t h = tab[i];
  h();
}
