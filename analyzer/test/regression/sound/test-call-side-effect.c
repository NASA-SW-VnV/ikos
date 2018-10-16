extern char* os_name(void);
extern int os_strlen(char*);

int main() {
  char* p = os_name();
  return os_strlen(p);
}
