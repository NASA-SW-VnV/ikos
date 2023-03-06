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
  MyStruct ss{};

  return ss._aa;
}
