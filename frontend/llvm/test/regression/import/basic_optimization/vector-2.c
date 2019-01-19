typedef double vector_t __attribute__((__vector_size__(16)));

vector_t a, b, c;

int main() {
  c = a * b;
  return 0;
}
