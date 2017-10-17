#include <ikos/domains/intervals.hpp>
#include <ikos/domains/pta.hpp>

using namespace ikos;

void test1() {
  // Sample function:
  //
  // int **f(int **p, int *q) {
  //   int *tmp = q + 1;
  //   *p = tmp;
  //   return p;
  // }

  index64_t f_uid = 128;
  pointer_var f_ptr = mk_pointer_var();
  pointer_var p = mk_pointer_var();
  pointer_var q = mk_pointer_var();
  pointer_var tmp = mk_pointer_var();

  pta_system s;

  // Pointer constraints
  s += f_ptr == mk_function_ref(f_uid);
  s += p == mk_param_ref(f_ptr, 1);
  s += q == mk_param_ref(f_ptr, 2);
  s += tmp == (q + z_interval(4, 4));
  s += (p + z_interval(0, 3)) << tmp;
  s += mk_return_ref(f_ptr) << p;
  s += p == mk_object_ref(32, z_interval(0, 4));
  s += q == mk_object_ref(33, z_interval(8, 16));

  std::cout << s << std::endl;
  s.solve();
  std::cout << "f_ptr = " << s.get(f_ptr) << std::endl;
  std::cout << "p = " << s.get(p) << std::endl;
  std::cout << "q = " << s.get(q) << std::endl;
  std::cout << "tmp = " << s.get(tmp) << std::endl;
}

void test2() {
  // int x, y;
  // int **p, *q;
  // q = &x;
  // p = &q;
  // *p = &y;

  // translated by llvm:
  // i32* x = alloca i32
  // i32* y = alloca i32
  // i32*** p = alloca i32**
  // i32** q = alloca i32*
  // store x, q
  // store q, p
  // tmp = load p
  // store y, tmp
  pointer_var x = mk_pointer_var();
  pointer_var y = mk_pointer_var();
  pointer_var p = mk_pointer_var();
  pointer_var q = mk_pointer_var();

  pta_system s;
  s += x == mk_object_ref(1, z_interval(0, 0));
  s += y == mk_object_ref(2, z_interval(0, 0));
  s += p == mk_object_ref(3, z_interval(0, 0));
  s += q == mk_object_ref(4, z_interval(0, 0));

  // store i32* %x, i32** %q
  s += (q + z_interval(0, 0)) << x;

  // store i32** %q, i32*** %p
  s += (p + z_interval(0, 0)) << q;

  // %1 = load i32*** %p
  pointer_var p_content = mk_pointer_var();
  s += p_content *= (p + z_interval(0, 0));

  // store i32* %y, i32** %1
  s += (p_content + z_interval(0, 0)) << y;

  // for debugging
  pointer_var q_content = mk_pointer_var();
  s += q_content *= (q + z_interval(0, 0));

  std::cout << s << std::endl;
  s.solve();
  std::cout << "*p = " << s.get(p_content) << std::endl;
  std::cout << "*q = " << s.get(q_content) << std::endl;
}

void test3() {
  // Sample function:
  // global struct foo * x = null;

  //-- %main._0 = call noalias i8* @malloc(i64 8) nounwind, !dbg !18
  // p0 = malloc (8);
  //-- %main._1 = bitcast i8* %main._0 to %struct.foo*, !dbg !18
  // p1 = p0;
  //-- store %struct.foo* %main._1, %struct.foo** @x, align 8, !dbg !18
  // *x = p1
  //-- %main._4 = load %struct.foo** @x, align 8, !dbg !21
  // p4 = *x;
  //-- %main._5 = getelementptr inbounds %struct.foo* %main._4, i32 0, i32 0,
  //! dbg !21
  // p5 = p4 + 0;
  //-- store i32 5, i32* %main._5, align 4, !dbg !21
  /*C*/
  // *p5 = 5; // x->a = 5;
  //-- %main._6 = load %struct.foo** @x, align 8, !dbg !22
  // p6 = *x;
  //-- %main._7 = load %struct.foo** @x, align 8, !dbg !22
  // p7 = *x;
  //-- %main._8 = getelementptr inbounds %struct.foo* %main._7, i32 0, i32 0,
  //! dbg !22
  // p8 = p7 + 0;
  //-- %main._9 = load i32* %main._8, align 4, !dbg !22
  /*C*/
  // p9 = *p8;  // *(x).a
  //-- %main._10 = add nsw i32 %main._9, 7, !dbg !22
  /*C*/
  // p10 = p9 + 7;
  //-- %main._11 = getelementptr inbounds %struct.foo* %main._6, i32 0, i32 1,
  //! dbg !22
  // p11 = p6 + 4;
  //--  store i32 %main._10, i32* %main._11, align 4, !dbg !22
  /*C*/
  // *p11 = p10; // x->b = x->a + 7;
  //
  // At this point x->a=5 and x->b=12
  //

  pointer_var x = mk_pointer_var();
  pointer_var p0 = mk_pointer_var();
  pointer_var p1 = mk_pointer_var();
  pointer_var p4 = mk_pointer_var();
  pointer_var p5 = mk_pointer_var();
  pointer_var p6 = mk_pointer_var();
  pointer_var p7 = mk_pointer_var();
  pointer_var p8 = mk_pointer_var();
  pointer_var p11 = mk_pointer_var();
  pta_system s;

  // Pointer constraints

  s += x ==
       mk_object_ref(34, z_interval(0, 0)); // declaration of global variable x
  s += p0 == mk_object_ref(25, z_interval(0, 0)); // malloc

  // p1 = p0;
  s += p1 == (p0 + z_interval(0, 0));
  // *x = p1;
  s += (x + z_interval(0, 7)) << p1;
  // p4 = *x;
  s += (p4 *= (x + z_interval(0, 0)));
  // p5 = p4;
  s += p5 == (p4 + z_interval(0, 0));
  // *p5 = 5;
  // p6 = *x;
  s += (p6 *= (x + z_interval(0, 0)));
  // p7 = *x;
  s += (p7 *= (x + z_interval(0, 0)));
  // p8 = p7;
  s += p8 == (p7 + z_interval(0, 0));
  // p9 = *p8;
  // p10 = p9 + 7;
  // p11  = p6 + 4;
  s += p11 == (p6 + z_interval(4, 4));
  // *p11 = p10;

  std::cout << s << std::endl;

  s.solve();
  std::cout << "p5 = " << s.get(p5) << std::endl;   //({25}, [0, 0])
  std::cout << "p8 = " << s.get(p8) << std::endl;   //({25}, [0, 0])
  std::cout << "p11 = " << s.get(p11) << std::endl; //({25}, [4, 4])
}

void test4() {
  /*
    l = malloc (sizeof(int) * ncols);
    for (i=0;i<10;i++)
    {
      l[i] = malloc (sizeof(int) * nrows)
    }
   */

  pointer_var l = mk_pointer_var();
  pointer_var tmp = mk_pointer_var();

  pta_system s;

  // Pointer constraints

  s += l == mk_object_ref(25, z_interval(10, 10)); // malloc
  s += tmp == mk_object_ref(34, z_interval(0, 0)); // malloc
  s += (l + z_interval(0, 9)) << tmp;

  std::cout << s << std::endl;

  s.solve();
  std::cout << "l = " << s.get(l) << std::endl; //({25}, [0, 0])
}

int main() {
  try {
    std::cout << "#######################################" << std::endl;
    test1();

    std::cout << "#######################################" << std::endl;
    test2();

    std::cout << "#######################################" << std::endl;
    test3();

    std::cout << "#######################################" << std::endl;
    test4();

  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}
