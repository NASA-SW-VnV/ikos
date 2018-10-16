/**
 * To generate LLVM select instruction, we need to compile
 * the code with optimization enabled (at least -O1)
 *
 *  > llvm-g++ -O1 -emit-llvm -o test.bc -g -c test.cpp
 */

int a;

int main() {
  return a > 0 ? 123 : 321;
}
