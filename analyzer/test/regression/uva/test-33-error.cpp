#include <memory>
int main(int argc, char *argv[])
{
  std::shared_ptr<bool> test1 (new bool);
  return (int)*test1;
}
