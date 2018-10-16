struct my_method {
  int (*div)(int x);
};

static int div(int x) {
  return 100 / x;
}

int main(int argc, char* argv[]) {
  static const struct my_method meth = {div};
  meth.div(0);
  return 0;
}
