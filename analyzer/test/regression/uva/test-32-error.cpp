#include <memory>

bool* getBool()
{
  return new bool;
}

int main(int argc, char *argv[])
{
  bool* test1 = new bool;
  bool* test2 = getBool();
  return (int)(*test1 && *test2);
}
