#include <memory>

struct MyStruct {
  int _aa;
  int _bb;
  MyStruct() noexcept
  {
    _aa = 5;
    _bb = 7;
  }
};

int main(int argc, char *argv[])
{
  // This test gives a false positive because the ikos AR
  // does not assure that the body is executed at least once.
  // That could probably be fixed.
  MyStruct* test1 = nullptr;
  for(int i = 0; i < 2;i++) {
    test1 = new MyStruct;
  }
  return test1->_aa;
}
