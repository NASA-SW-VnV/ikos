#include <iostream>

#include <ikos/common/types.hpp>
#include <ikos/domains/discrete_domains.hpp>

using namespace std;
using namespace ikos;

typedef string_factory::indexed_string varname_t;
typedef discrete_domain< varname_t > discrete_domain_t;

int main() {
  string_factory vfac;

  discrete_domain_t s1, s2, s3, r;

  s1 = discrete_domain_t::bottom();
  s2 = discrete_domain_t::bottom();

  s1 += vfac["a"];
  s1 += vfac["b"];
  s1 += vfac["c"];
  s1 += vfac["d"];

  s2 += vfac["c"];
  s2 += vfac["d"];
  s2 += vfac["e"];
  s2 += vfac["f"];

  r = s1 | s2;
  cout << s1 << " | " << s2 << " = " << r << endl;
  r = s1 | s3;
  cout << s1 << " | " << s3 << " = " << r << endl;
  r = s1 & s3;
  cout << s1 << " & " << s3 << " = " << r << endl;
  r = s1 & s2;
  cout << s1 << " & " << s2 << " = " << r << endl;
  cout << "#" << s1 << " = " << s1.size() << endl;
  cout << "#" << r << " = " << r.size() << endl;
}
