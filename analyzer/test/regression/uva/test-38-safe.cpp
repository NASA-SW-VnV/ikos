#include <memory>
using namespace std;
int main(int argc, char *argv[])
{
  auto test1 = make_unique<bool>();
  return (int)*test1;
}
