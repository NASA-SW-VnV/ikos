extern char* os_name(void);

int main() {
  char* p = os_name();
  *p = 'a';
  return 0;
}
