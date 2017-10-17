#include <iostream>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/octagons.hpp>
#include <ikos/domains/uninitialized.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/domains/array_graph.hpp>

using namespace ikos;
using namespace analyzer;

typedef variable< z_number, varname_t > var_t;
typedef uninitialized_domain_impl< varname_t > uninitialized_domain_t;
typedef linear_constraint< z_number, varname_t > linear_constraint_t;
typedef linear_constraint_system< z_number, varname_t >
    linear_constraint_system_t;
typedef linear_expression< z_number, varname_t > linear_expression_t;
typedef interval_domain< z_number, varname_t > interval_domain_t;
typedef octagon< z_number, varname_t > scalar_domain_t;

typedef array_graph< varname_t, uninitialized_domain_t, scalar_domain_t >
    array_graph_t;
typedef array_graph_domain< scalar_domain_t,
                            z_number,
                            varname_t,
                            uninitialized_domain_t > array_graph_domain_t;

/////////////////////////////////////////////////////////////////////////
/// TESTS FOR ARRAY_GRAPH ///////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void test_0() {
  std::cout << "TEST #0: array graph " << std::endl;
  VariableFactory vfac;
  varname_t v_A = vfac["A"];
  varname_t v_0 = vfac["0"];
  varname_t v_i = vfac["i"];
  varname_t v_n = vfac["n"];

  array_graph_t g = array_graph_t::top();
  g.insert_vertex(v_0);
  g.insert_vertex(v_i);
  g.insert_vertex(v_n);

  uninitialized_domain_t inv1 = uninitialized_domain_t::top();
  inv1.set(v_A, uninitialized_value::initialized());

  uninitialized_domain_t inv2 = uninitialized_domain_t::bottom();
  g.update_weight(v_0, v_i, inv1);
  g.update_weight(v_i, v_n, inv2);
  g.update_weight(v_n, v_i, inv2);

  std::cout << g << std::endl;
}

void test_1() {
  std::cout << "TEST #1: array_graph reduce " << std::endl;
  VariableFactory vfac;
  varname_t v_A = vfac["A"];
  varname_t v_0 = vfac["0"];
  varname_t v_i = vfac["i"];
  varname_t v_i1 = vfac["i+"];
  varname_t v_n = vfac["n"];

  array_graph_t g = array_graph_t::top();
  g.insert_vertex(v_0);
  g.insert_vertex(v_i);
  g.insert_vertex(v_i1);
  g.insert_vertex(v_n);

  uninitialized_domain_t inv1 = uninitialized_domain_t::top();
  inv1.set(v_A, uninitialized_value::initialized());

  uninitialized_domain_t inv2 = uninitialized_domain_t::bottom();
  g.update_weight(v_0, v_i, inv1);
  g.update_weight(v_i, v_i1, inv1);
  g.update_weight(v_i1, v_i, inv2);

  std::cout << g << std::endl;

  scalar_domain_t inv;
  inv += linear_constraint_t(var_t(v_0) == 0);
  inv += linear_constraint_t(var_t(v_i) >= var_t(v_0));
  inv += linear_constraint_t(var_t(v_n) == 11);
  inv += linear_constraint_t(var_t(v_n) >= var_t(v_0) + 1);
  inv += linear_constraint_t(var_t(v_i) <= var_t(v_n) - 1);
  inv += linear_constraint_t(var_t(v_i1) == var_t(v_i) + 1);

  std::cout << "After reduction with " << inv << std::endl;
  g.reduce(inv);
  std::cout << g << std::endl;
}

void test_2() {
  std::cout << "TEST #2: array_graph forget " << std::endl;
  VariableFactory vfac;
  varname_t v_A = vfac["A"];
  varname_t v_0 = vfac["0"];
  varname_t v_i = vfac["i"];
  varname_t v_n = vfac["n"];

  uninitialized_domain_t inv_top = uninitialized_domain_t::top();
  uninitialized_domain_t inv_bot = uninitialized_domain_t::bottom();
  uninitialized_domain_t inv1 = uninitialized_domain_t::top();
  inv1.set(v_A, uninitialized_value::initialized());
  uninitialized_domain_t inv2 = uninitialized_domain_t::top();
  inv2.set(v_A, uninitialized_value::uninitialized());

  array_graph_t g1(array_graph_t::top());
  g1.insert_vertex(v_0);
  g1.insert_vertex(v_i);
  g1.insert_vertex(v_n);
  g1.update_weight(v_0, v_i, inv1);
  g1.update_weight(v_i, v_n, inv2);
  g1.update_weight(v_0, v_n, inv_top);
  g1.update_weight(v_i, v_0, inv_bot);
  g1.update_weight(v_n, v_i, inv_bot);
  g1.update_weight(v_n, v_0, inv_bot);

  array_graph_t g2(g1);
  std::cout << "g1:" << g1 << std::endl;
  std::cout << "g2:" << g2 << std::endl;

  g2 -= v_i;
  std::cout << "After removing " << v_i << " from g2:" << std::endl;
  std::cout << "g1:" << g1 << std::endl;
  std::cout << "g2:" << g2 << std::endl;
}

void test_3() {
  std::cout << "TEST #3: array_graph <=, |, and & " << std::endl;
  VariableFactory vfac;
  varname_t v_A = vfac["A"];
  varname_t v_0 = vfac["0"];
  varname_t v_i = vfac["i"];
  varname_t v_n = vfac["n"];

  uninitialized_domain_t inv_top = uninitialized_domain_t::top();
  uninitialized_domain_t inv_bot = uninitialized_domain_t::bottom();
  uninitialized_domain_t inv1 = uninitialized_domain_t::top();
  inv1.set(v_A, uninitialized_value::initialized());
  uninitialized_domain_t inv2 = uninitialized_domain_t::top();
  inv2.set(v_A, uninitialized_value::uninitialized());

  array_graph_t g1(array_graph_t::top());
  g1.insert_vertex(v_0);
  g1.insert_vertex(v_i);
  g1.insert_vertex(v_n);
  g1.update_weight(v_0, v_i, inv1);
  g1.update_weight(v_i, v_n, inv2);
  g1.update_weight(v_0, v_n, inv_top);
  g1.update_weight(v_i, v_0, inv_bot);
  g1.update_weight(v_n, v_i, inv_bot);
  g1.update_weight(v_n, v_0, inv_bot);

  array_graph_t g2(array_graph_t::top());
  g2.insert_vertex(v_0);
  g2.insert_vertex(v_i);
  g2.insert_vertex(v_n);

  std::cout << "g1:" << g1 << std::endl;
  std::cout << "g2:" << g2 << std::endl;
  bool f1 = (g1 <= g2);
  bool f2 = (g2 <= g1);
  std::cout << "(g1 <= g2): " << f1 << std::endl;
  std::cout << "(g2 <= g1): " << f2 << std::endl;
  array_graph_t g3 = g1 | g2;
  std::cout << "join(g1,g2): " << g3 << std::endl;
  array_graph_t g4(g1 & g2);
  std::cout << "meet(g1,g2): " << g4 << std::endl;
}

void test_4a() {
  std::cout << "TEST #4a: array_graph forget " << std::endl;
  VariableFactory vfac;
  varname_t v_0 = vfac["0"];
  varname_t v_i = vfac["i"];
  varname_t v_n = vfac["n"];
  varname_t v_j = vfac["j"];

  array_graph_t g1(array_graph_t::top());
  g1.insert_vertex(v_0);
  g1.insert_vertex(v_i);
  g1.insert_vertex(v_n);
  std::cout << "g1:" << g1 << std::endl;
  g1 -= v_i;
  std::cout << "g1 after removing " << v_i << ":" << g1 << std::endl;

  array_graph_t g2(array_graph_t::top());
  g2.insert_vertex(v_0);
  g2.insert_vertex(v_i);
  g2.insert_vertex(v_j);
  g2.insert_vertex(v_n);
  std::cout << "g2:" << g2 << std::endl;
  g2 -= v_i;
  std::cout << "g2 after removing " << v_i << ":" << g2 << std::endl;
  g2 -= v_j;
  std::cout << "g2 after removing " << v_i << " and " << v_j << ":" << g2
            << std::endl;
}

void test_4b() {
  std::cout << "TEST #4b: array_graph forget " << std::endl;
  VariableFactory vfac;
  varname_t v_0 = vfac["0"];
  varname_t v_i = vfac["i"];
  varname_t v_n = vfac["n"];
  varname_t v_A = vfac["A"];

  array_graph_t g(array_graph_t::top());
  g.insert_vertex(v_0);
  g.insert_vertex(v_i);
  g.insert_vertex(v_n);

  uninitialized_domain_t w = uninitialized_domain_t::top();
  w.set(v_A, uninitialized_value::initialized());
  g.update_weight(v_0, v_n, w);

  std::cout << g << std::endl;
  g -= v_i;
  std::cout << "after removing " << v_i << ":" << g << std::endl;
}

////////////////////////////////////////////////////////////////////////////////
/// TESTS FOR ARRAY_GRAPH_DOMAIN ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void test_5() {
  std::cout << "TEST #5: scalar domain x array_graph " << std::endl;
  VariableFactory vfac;
  array_graph_domain_t inv;
  varname_t vA = vfac["A"];
  varname_t vB = vfac["B"];
  varname_t v0 = vfac["0"];
  varname_t vi = vfac["i"];
  varname_t vj = vfac["j"];
  varname_t vn = vfac["n"];

  uninitialized_domain_t w1 = uninitialized_domain_t::top();
  w1.set(vA, uninitialized_value::initialized());
  uninitialized_domain_t w2 = uninitialized_domain_t::top();
  w2.set(vB, uninitialized_value::initialized());

  inv.meet_weight(0, vi, w1, vfac);
  inv.meet_weight(0, vi, w2, vfac);
  inv.meet_weight(vi, vn, w2, vfac);
  std::cout << "Before adding constraints: " << std::endl << inv << std::endl;

  linear_constraint_system_t csts;
  csts += linear_constraint_t(var_t(vi) >= 0);
  csts += linear_constraint_t(var_t(vj) >= 0);
  csts += linear_constraint_t(var_t(vi) <= var_t(vn) - 1);
  csts += linear_constraint_t(var_t(vj) <= var_t(vn) - 1);
  inv.assertion(csts, vfac);
  std::cout << "After adding constraints: " << std::endl << inv << std::endl;
}

void test_6() {
  std::cout
      << "TEST #6: (scalar domain x array_graph) load/store, assign, forget "
      << std::endl;
  VariableFactory vfac;
  varname_t vA = vfac["A"];
  varname_t vi = vfac["i"];
  varname_t vn = vfac["n"];

  uninitialized_domain_t w = uninitialized_domain_t::top();
  w.set(vA, uninitialized_value::initialized());

  array_graph_domain_t inv = array_graph_domain_t::top();
  linear_constraint_system_t csts;
  csts += linear_constraint_t(var_t(vi) >= 0);
  csts += linear_constraint_t(var_t(vi) <= var_t(vn) - 1);
  inv.assertion(csts, vfac);
  inv.meet_weight(0, 1, w, vfac);  // ad-hoc
  inv.meet_weight(1, vi, w, vfac); // ad-hoc
  std::cout << inv << std::endl;

  // v = A[i]
  uninitialized_domain_t x = inv[vi];
  uninitialized_value v = x[vA];
  std::cout << "array read  " << vA << "[" << vi << "]"
            << " = " << v << std::endl;
  // A[i] = v
  inv.store(vi, w);
  std::cout << "array write " << vA << "[" << vi << "]"
            << ":= \"initialized\"" << std::endl;
  // v = A[i]
  x = inv[vi];
  v = x[vA];
  std::cout << "array read  " << vA << "[" << vi << "]"
            << " = " << v << std::endl;

  varname_t vj = vfac["j"];
  inv.assign(vj, linear_expression_t(vi), vfac);
  std::cout << vj << ":=" << vi << std::endl << inv << std::endl;
  inv -= vi;
  std::cout << "after removing " << vi << std::endl << inv << std::endl;
}

void test_7() {
  try {
    std::cout << "TEST #7: (scalar domain x array_graph) arithmetic operations "
              << std::endl;
    VariableFactory vfac;
    varname_t vA = vfac["A"];
    varname_t vi = vfac["i"];
    varname_t vn = vfac["n"];

    array_graph_domain_t inv = array_graph_domain_t::top();
    linear_constraint_system_t csts;

    csts += linear_constraint_t(var_t(vi) >= 0);
    csts += linear_constraint_t(var_t(vn) == 10);
    csts += linear_constraint_t(var_t(vi) <= var_t(vn) - 1);
    inv.assertion(csts, vfac);
    uninitialized_domain_t w = uninitialized_domain_t::top();
    w.set(vA, uninitialized_value::initialized());

    inv.meet_weight(z_number(0), vi, w, vfac); // ad-hoc
    inv.store(vi, w);

    std::cout << "Before operations: " << inv << std::endl;
    array_graph_domain_t tmp3(inv);
    std::cout << vi << ":=" << vi << " + 0" << std::endl;
    tmp3.apply(OP_ADDITION, vi, z_number(0), vfac);
    std::cout << tmp3 << std::endl;
    array_graph_domain_t tmp1(inv);
    std::cout << vi << ":=" << vi << " + 1" << std::endl;
    tmp1.apply(OP_ADDITION, vi, z_number(1), vfac);
    std::cout << tmp1 << std::endl;
    array_graph_domain_t tmp2(inv);
    std::cout << vi << ":=" << vi << " + 2" << std::endl;
    tmp2.apply(OP_ADDITION, vi, z_number(2), vfac);
    std::cout << tmp2 << std::endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void test_8() {
  try {
    std::cout
        << "TEST #8: (scalar domain x array_graph) more arithmetic operations "
        << std::endl;
    VariableFactory vfac;
    varname_t vA = vfac["A"];
    varname_t vi = vfac["i"];
    varname_t vj = vfac["j"];
    varname_t vn = vfac["n"];

    array_graph_domain_t inv = array_graph_domain_t::top();
    linear_constraint_system_t csts;
    csts += linear_constraint_t(var_t(vi) >= 0);
    csts += linear_constraint_t(var_t(vn) == 15);
    csts += linear_constraint_t(var_t(vi) <= 9);
    csts += linear_constraint_t(var_t(vj) == var_t(vi) + 2);
    inv.assertion(csts, vfac);

    uninitialized_domain_t w = uninitialized_domain_t::top();
    w.set(vA, uninitialized_value::initialized());

    inv.meet_weight(z_number(0), vi, w, vfac); // ad-hoc
    inv.meet_weight(vi, vj, w, vfac);          // ad-hoc
    inv.store(vi, w);

    std::cout << "Before operations:" << inv << std::endl;
    array_graph_domain_t tmp3(inv);
    std::cout << vi << ":=" << vi << " + 0" << std::endl;
    tmp3.apply(OP_ADDITION, vi, z_number(0), vfac);
    // tmp3 -= vj;
    std::cout << tmp3 << std::endl;
    array_graph_domain_t tmp1(inv);
    std::cout << vi << ":=" << vi << " + 1" << std::endl;
    tmp1.apply(OP_ADDITION, vi, z_number(1), vfac);
    // tmp1 -= vj;
    std::cout << tmp1 << std::endl;
    array_graph_domain_t tmp2(inv);
    std::cout << vi << ":=" << vi << " + 2" << std::endl;
    tmp2.apply(OP_ADDITION, vi, z_number(2), vfac);
    // tmp2 -= vj;
    std::cout << tmp2 << std::endl;
    array_graph_domain_t tmp4(inv);
    std::cout << vi << ":=" << vi << " + 4" << std::endl;
    tmp4.apply(OP_ADDITION, vi, z_number(3), vfac);
    // tmp4.forget(vj);
    std::cout << tmp4 << std::endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void test_9() {
  try {
    std::cout << "TEST #9: (scalar domain x array_graph) substraction "
              << std::endl;
    VariableFactory vfac;
    varname_t vA = vfac["A"];
    varname_t vi = vfac["i"];
    varname_t vk = vfac["k"];
    varname_t vn = vfac["n"];

    array_graph_domain_t inv = array_graph_domain_t::top();
    linear_constraint_system_t csts;
    csts += linear_constraint_t(var_t(vi) >= 5);
    csts += linear_constraint_t(var_t(vn) == 10);
    csts += linear_constraint_t(var_t(vi) <= 9);
    csts += linear_constraint_t(var_t(vk) == var_t(vi) - 1);
    inv.assertion(csts, vfac);
    uninitialized_domain_t w = uninitialized_domain_t::top();
    w.set(vA, uninitialized_value::initialized());

    inv.meet_weight(vi, vn, w, vfac); // ad-hoc
    inv.meet_weight(vk, vi, w, vfac); // ad-hoc
    std::cout << "Before operations:" << inv << std::endl;
    inv.apply(OP_SUBTRACTION, vi, z_number(1), vfac);
    inv -= vk;
    std::cout << vi << ":=" << vi << " - 1" << std::endl << inv << std::endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void test_10() {
  try {
    std::cout << "TEST #10: (scalar domain x array_graph) substraction "
              << std::endl;
    VariableFactory vfac;
    varname_t vA = vfac["A"];
    varname_t vi = vfac["i"];
    varname_t vk = vfac["k"];

    array_graph_domain_t inv = array_graph_domain_t::top();
    linear_constraint_system_t csts;
    csts += linear_constraint_t(var_t(vi) >= 1);
    inv.assertion(csts, vfac);
    uninitialized_domain_t w = uninitialized_domain_t::top();
    w.set(vA, uninitialized_value::initialized());
    inv.meet_weight(0, vi, w, vfac); // ad-hoc
    std::cout << "Before operations:" << inv << std::endl;
    inv.apply(OP_SUBTRACTION, vk, vi, z_number(1), vfac);
    std::cout << vk << ":=" << vi << " - 1" << std::endl << inv << std::endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

void test_11() {
  try {
    std::cout << "TEST #11 " << std::endl;
    VariableFactory vfac;
    varname_t vi = vfac["i"];
    varname_t vn = vfac["n"];
    varname_t vA = vfac["A"];
    ///////////////////////////////////////////////////////
    // 1st
    ///////////////////////////////////////////////////////
    array_graph_domain_t inv1 = array_graph_domain_t::top();
    /// assume n>0
    inv1.assertion(linear_constraint_t(var_t(vn) >= 1), vfac);
    /// i = 0
    inv1.assign(vi, 0, vfac);
    /// i < n
    inv1.assertion(linear_constraint_t(var_t(vi) <= var_t(vn) - 1), vfac);
    /// A[i] = 5;

    uninitialized_domain_t w;
    w.set(vA, uninitialized_value::initialized());
    inv1.store(vi, w);
    /// i++
    inv1.apply(OP_ADDITION, vi, 1, vfac);
    std::cout << "after 1st iteration: " << std::endl << inv1 << std::endl;
    ///////////////////////////////////////////////////////
    // 2nd
    ///////////////////////////////////////////////////////
    array_graph_domain_t inv2(inv1);
    /// i < n
    inv2.assertion(linear_constraint_t(var_t(vi) <= var_t(vn) - 1), vfac);
    /// A[i] = 5;
    inv2.store(vi, w);
    /// i++
    inv2.apply(OP_ADDITION, vi, 1, vfac);
    std::cout << "after 2nd iteration: " << std::endl << inv2 << std::endl;
    array_graph_domain_t inv3 = inv1 || inv2;
    std::cout << "Widening 1st and 2nd: " << std::endl << inv3 << std::endl;
    ///////////////////////////////////////////////////////
    // 3rd
    ///////////////////////////////////////////////////////
    /// i < n
    inv3.assertion(linear_constraint_t(var_t(vi) <= var_t(vn) - 1), vfac);
    /// A[i] = 5;
    inv3.store(vi, w);
    /// i++
    inv3.apply(OP_ADDITION, vi, 1, vfac);
    std::cout << "after 3rd iteration: " << std::endl << inv3 << std::endl;
    // ///////////////////////////////////////////////////////
    // // 4th join with initial state
    // ///////////////////////////////////////////////////////
    // array_graph_domain_t inv4 = array_graph_domain_t::top();
    // /// i = 0
    // inv4.assign(vi, 0, vfac);
    // inv4.assertion(linear_constraint_t( var_t(vn) >= 1), vfac);
    // array_graph_domain_t inv5(inv3 | inv4);
    // std::cout << "after join with initial state: " << std::endl << inv5 <<
    // std::endl;
    // ///////////////////////////////////////////////////////
    // // 5th
    // ///////////////////////////////////////////////////////
    // array_graph_domain_t inv6(inv5);
    // /// i >= n
    // inv6.assertion(linear_constraint_t( var_t(vi) >= var_t(vn)), vfac);
    // inv6 -= vi;
    // std::cout << "End: "<< inv6 << std::endl;
  } catch (ikos::exception& e) {
    std::cerr << e.what() << std::endl;
  }
}

int main(int argc, char** argv) {
  test_0();
  test_1();
  test_2();
  test_3();
  test_4a();
  test_4b();
  test_5();
  test_6();
  test_7();
  test_8();
  test_9();
  test_10();
  test_11();

  return 1;
}
