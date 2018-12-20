extern int f();

int g() noexcept {
  return f();
}
