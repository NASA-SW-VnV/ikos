#include <memory>
using namespace std;

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
  auto test1 = make_shared<MyStruct>();
  
  return test1->_aa;
}
