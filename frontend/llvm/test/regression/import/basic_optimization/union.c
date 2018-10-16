union my_union {
  int m_int;
  char* m_ptr;
};

int main() {
  union my_union x = {.m_int = 1};
}
