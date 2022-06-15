#include <memory>

struct MyStruct {
  int _aa;
  int _bb;
  MyStruct() noexcept
  {
    //_aa = 5;
    _bb = 7;
  }
};

int main(int argc, char *argv[])
{
    MyStruct* test1 = nullptr;
    int i = 0;
    do {
        test1 = new MyStruct;
        i++;
    }while (i < 2);
    return test1->_aa;
}
