#include <memory>
int main(int argc, char *argv[])
{
  std::unique_ptr<bool> test1 (new bool);
  return (int)*test1;
}
