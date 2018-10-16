// SAFE
// from King and Simon

void main() {
  char s[32] = "the string";
  int i = 0;
  char c;
  while (1) {
    c = s[i];
    if (c == 0)
      break;
    i = i + 1;
  }
}
