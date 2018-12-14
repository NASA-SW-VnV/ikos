int f(decltype(nullptr) x) {
  return 1;
}

int main() {
  return f(nullptr);
}
