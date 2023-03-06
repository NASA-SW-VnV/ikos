#include <memory>
using namespace std;
int main(int argc, char *argv[])
{
  auto test1 = make_shared<bool>();
  //dereference pointer that is properly initialized.
  return (int)*test1;
}
