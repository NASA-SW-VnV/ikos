struct MyStruct {
  bool is_down[3] = {};
};

int main(int argc, char* argv[]) {
  MyStruct s;
  s.is_down[2] = false;
  return 0;
}
