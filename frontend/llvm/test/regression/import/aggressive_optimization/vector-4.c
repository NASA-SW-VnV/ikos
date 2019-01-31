typedef long vector_t __attribute__((__vector_size__(16)));

vector_t f(vector_t x) {
  return __builtin_ia32_pshufd(x, 0);
}
