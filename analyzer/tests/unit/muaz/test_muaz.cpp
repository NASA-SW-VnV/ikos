/* Test different domains and analyses over a toy CFG */

#include <iostream>

#include <ikos/algorithms/linear_constraints.hpp>
#include <ikos/domains/cpa.hpp>
#include <ikos/domains/intervals.hpp>
#include <ikos/domains/octagons.hpp>
#include <ikos/domains/uninitialized.hpp>
#include <ikos/iterators/fwd_fixpoint_iterators.hpp>
#include <ikos/number/z_number.hpp>

#include <analyzer/analysis/common.hpp>
#include <analyzer/analysis/liveness.hpp>
#include <analyzer/domains/uninitialized_array.hpp>
#include <analyzer/examples/muaz.hpp>
#include <analyzer/ikos-wrapper/iterators.hpp>

using namespace ikos;
using namespace ikos::muaz;
using namespace analyzer;

// #define PRINT_PP
typedef ikos::variable< ikos::z_number, varname_t > z_var;
typedef muaz_cfg< varname_t, std::string > cfg_t;
typedef cfg_t::basic_block_id_t basic_block_id_t;
typedef cfg_t::basic_block_t basic_block_t;
typedef cfg_t::statement_t statement_t;
typedef statement_visitor< varname_t, std::string > visitor_t;
typedef linear_expression< ikos::z_number, varname_t > linear_expression_t;
typedef linear_expression_t::variable_set_t variable_set_t;
// numerical domains
typedef interval_domain< ikos::z_number, varname_t > interval_domain_t;
typedef octagon< ikos::z_number, varname_t > octagon_domain_t;
// scalar uva
typedef uninitialized_domain_impl< varname_t > uva_domain_t;
// scalar+array uva
typedef uninitialized_array_domain< octagon_domain_t,
                                    ikos::z_number,
                                    varname_t > uva_array_domain_t;
// liveness
typedef liveness_discrete_impl::liveness_domain< varname_t > liveness_domain_t;
// cpa
typedef cpa_domain< varname_t, ikos::z_number > cpa_domain_t;

namespace stmt_analyzer_impl {
template < typename abs_domain_t >
inline void visit(abs_domain_t& inv,
                  typename visitor_t::z_binary_operation_t& stmt,
                  VariableFactory&);
template < typename abs_domain_t >
inline void visit(abs_domain_t& inv,
                  typename visitor_t::z_linear_assignment_t& stmt,
                  VariableFactory&);
template < typename abs_domain_t >
inline void visit(abs_domain_t& inv,
                  typename visitor_t::z_linear_assertion_t& stmt,
                  VariableFactory&);
template < typename abs_domain_t >
inline void visit(abs_domain_t& inv,
                  typename visitor_t::z_array_write_t& stmt,
                  VariableFactory&);
template < typename abs_domain_t >
inline void visit(abs_domain_t& inv,
                  typename visitor_t::z_array_read_t& stmt,
                  VariableFactory&);
}; // end namespace stmt_analyzer_impl

template < typename abs_domain_t >
class stmt_analyzer : public visitor_t {
private:
  abs_domain_t _inv;
  VariableFactory& _vfac;

public:
  stmt_analyzer(abs_domain_t inv, VariableFactory& vfac)
      : _inv(inv), _vfac(vfac) {}

  abs_domain_t inv() { return this->_inv; }

  void visit(z_binary_operation_t& stmt) {
    stmt_analyzer_impl::visit< abs_domain_t >(this->_inv, stmt, _vfac);
  }
  void visit(z_linear_assignment_t& stmt) {
    stmt_analyzer_impl::visit< abs_domain_t >(this->_inv, stmt, _vfac);
  }
  void visit(z_linear_assertion_t& stmt) {
    stmt_analyzer_impl::visit< abs_domain_t >(this->_inv, stmt, _vfac);
  }
  void visit(z_array_write_t& stmt) {
    stmt_analyzer_impl::visit< abs_domain_t >(this->_inv, stmt, _vfac);
  }
  void visit(z_array_read_t& stmt) {
    stmt_analyzer_impl::visit< abs_domain_t >(this->_inv, stmt, _vfac);
  }
  void visit(checkpoint_t&) {}
}; // class stmt_analyzer

template < typename abs_domain_t >
class fwd_analyzer : public interleaved_fwd_fixpoint_iterator< basic_block_id_t,
                                                               cfg_t,
                                                               abs_domain_t > {
public:
  typedef interleaved_fwd_fixpoint_iterator< basic_block_id_t,
                                             cfg_t,
                                             abs_domain_t > fwd_iterator_t;

private:
  typedef stmt_analyzer< abs_domain_t > stmt_analyzer_t;
  VariableFactory& _vfac;

public:
  fwd_analyzer(cfg_t cfg, VariableFactory& vfac)
      : fwd_iterator_t(cfg), _vfac(vfac) {}

  void process_pre(basic_block_id_t node, abs_domain_t inv) {
#ifdef PRINT_PP
    std::cout << node << std::endl;
    basic_block_t b = this->get_cfg().get_node(node);
    stmt_analyzer_t a(inv, _vfac);
    for (basic_block_t::iterator it = b.begin(); it != b.end(); ++it) {
      std::cout << "\tIN(";
      it->write(std::cout);
      abs_domain_t cur_inv = a.inv();
      std::cout << "): " << cur_inv << std::endl;
      it->accept(&a);
    }
#else
    std::cout << "Pre at " << node << ": " << inv << std::endl;
#endif
  }

  void process_post(basic_block_id_t node, abs_domain_t inv) {
    std::cout << "Post at " << node << ": " << inv << std::endl;
  }

  abs_domain_t analyze(basic_block_id_t node, abs_domain_t pre) {
    basic_block_t b = this->get_cfg().get_node(node);
    stmt_analyzer_t a(pre, _vfac);
    for (basic_block_t::iterator it = b.begin(); it != b.end(); ++it) {
      it->accept(&a);
    }

    return a.inv();
  }
}; // class fwd_analyzer

template < typename abs_domain_t >
class backward_analyzer : public backward_fixpoint_iterator< basic_block_id_t,
                                                             cfg_t,
                                                             abs_domain_t > {
private:
  typedef backward_fixpoint_iterator< basic_block_id_t, cfg_t, abs_domain_t >
      backward_fixpoint_iterator_t;

private:
  typedef stmt_analyzer< abs_domain_t > stmt_analyzer_t;

private:
  VariableFactory& _vfac;

public:
  backward_analyzer(cfg_t cfg, VariableFactory& vfac)
      : backward_fixpoint_iterator_t(cfg, true), _vfac(vfac) {}

  abs_domain_t analyze(basic_block_id_t node, abs_domain_t post) {
    basic_block_t b = this->get_cfg().get_node(node);
    stmt_analyzer_t a(post, _vfac);
    for (basic_block_t::iterator it = b.begin(); it != b.end(); ++it) {
      it->accept(&a);
    }
    return a.inv();
  }

  void check_post(basic_block_id_t node, abs_domain_t post) {}

  void check_pre(basic_block_id_t node, abs_domain_t pre) {
    std::cout << "Pre at " << node << ": " << pre << std::endl;
  }

}; // end backward_analyzer

// octagons
namespace stmt_analyzer_impl {
template <>
inline void visit(octagon_domain_t& inv,
                  visitor_t::z_binary_operation_t& stmt,
                  VariableFactory&) {
  inv.apply(stmt.operation(),
            stmt.lhs().name(),
            stmt.left_operand().name(),
            stmt.right_operand().name());
}

template <>
inline void visit(octagon_domain_t& inv,
                  visitor_t::z_linear_assignment_t& stmt,
                  VariableFactory&) {
  inv.assign(stmt.lhs().name(), linear_expression_t(stmt.rhs()));
}

template <>
inline void visit(octagon_domain_t& inv,
                  visitor_t::z_linear_assertion_t& stmt,
                  VariableFactory&) {
  inv += stmt.constraint();
}

template <>
inline void visit(octagon_domain_t& inv,
                  visitor_t::z_array_write_t& stmt,
                  VariableFactory&) {}

template <>
inline void visit(octagon_domain_t& inv,
                  visitor_t::z_array_read_t& stmt,
                  VariableFactory&) {}

}; // end namespace stmt_analyzer_impl

// intervals
namespace stmt_analyzer_impl {

template <>
inline void visit(interval_domain_t& inv,
                  visitor_t::z_binary_operation_t& stmt,
                  VariableFactory&) {
  inv.apply(stmt.operation(),
            stmt.lhs().name(),
            stmt.left_operand().name(),
            stmt.right_operand().name());
}

template <>
inline void visit(interval_domain_t& inv,
                  visitor_t::z_linear_assignment_t& stmt,
                  VariableFactory&) {
  inv.assign(stmt.lhs().name(), linear_expression_t(stmt.rhs()));
}

template <>
inline void visit(interval_domain_t& inv,
                  visitor_t::z_linear_assertion_t& stmt,
                  VariableFactory&) {
  inv += stmt.constraint();
}

template <>
inline void visit(interval_domain_t& inv,
                  visitor_t::z_array_write_t& stmt,
                  VariableFactory&) {}

template <>
inline void visit(interval_domain_t& inv,
                  visitor_t::z_array_read_t& stmt,
                  VariableFactory&) {}

}; // end namespace stmt_analyzer_impl

// scalar uva
namespace stmt_analyzer_impl {
template <>
inline void visit(uva_domain_t& inv,
                  visitor_t::z_binary_operation_t& stmt,
                  VariableFactory&) {
  inv.assign_uninitialized(stmt.lhs().name(),
                           stmt.left_operand().name(),
                           stmt.right_operand().name());
}

template <>
inline void visit(uva_domain_t& inv,
                  visitor_t::z_linear_assignment_t& stmt,
                  VariableFactory&) {
  linear_expression_t e = stmt.rhs();
  if (e.is_constant()) {
    inv.set(stmt.lhs().name(), uninitialized_value::initialized());
  } else {
    std::vector< varname_t > varnames;
    for (linear_expression_t::iterator it = e.begin(), et = e.end(); it != et;
         ++it) {
      varnames.push_back(it->second.name());
    }
    inv.assign_uninitialized(stmt.lhs().name(), varnames);
  }
}
template <>
inline void visit(uva_domain_t& inv,
                  visitor_t::z_linear_assertion_t& stmt,
                  VariableFactory&) {}

template <>
inline void visit(uva_domain_t& inv,
                  visitor_t::z_array_write_t& stmt,
                  VariableFactory&) {}

template <>
inline void visit(uva_domain_t& inv,
                  visitor_t::z_array_read_t& stmt,
                  VariableFactory&) {
  z_var lhs = stmt.lhs();
  inv.set(lhs.name(), uninitialized_value::top());
}

}; // end namespace stmt_analyzer_impl

// scalar+array uva
namespace stmt_analyzer_impl {
template <>
inline void visit(uva_array_domain_t& inv,
                  visitor_t::z_binary_operation_t& stmt,
                  VariableFactory& vfac) {
  inv.apply(stmt.operation(),
            stmt.lhs().name(),
            stmt.left_operand().name(),
            stmt.right_operand().name(),
            vfac);
}

template <>
inline void visit(uva_array_domain_t& inv,
                  visitor_t::z_linear_assignment_t& stmt,
                  VariableFactory& vfac) {
  inv.assign(stmt.lhs().name(), linear_expression_t(stmt.rhs()), vfac);
}

template <>
inline void visit(uva_array_domain_t& inv,
                  visitor_t::z_linear_assertion_t& stmt,
                  VariableFactory& vfac) {
  inv.assertion(stmt.constraint(), vfac);
}

template <>
inline void visit(uva_array_domain_t& inv,
                  visitor_t::z_array_write_t& stmt,
                  VariableFactory&) {
  if (boost::optional< z_var > idx = stmt.index().get_variable()) {
    inv.array_write(stmt.array().name(), (*idx).name(), stmt.val());
  } else
    assert(false && "array indexes can be only variables");
}

template <>
inline void visit(uva_array_domain_t& inv,
                  visitor_t::z_array_read_t& stmt,
                  VariableFactory&) {
  if (boost::optional< z_var > idx = stmt.index().get_variable()) {
    inv.array_read(stmt.lhs().name(), stmt.array().name(), (*idx).name());
  } else
    assert(false && "array indexes can be only variables");
}
}; // end namespace stmt_analyzer_impl

// liveness
namespace stmt_analyzer_impl {
template <>
inline void visit(liveness_domain_t& inv,
                  visitor_t::z_binary_operation_t& stmt,
                  VariableFactory&) {
  // KILL
  inv -= stmt.lhs().name();
  // GEN
  inv += stmt.left_operand().name();
  inv += stmt.right_operand().name();
}

template <>
inline void visit(liveness_domain_t& inv,
                  visitor_t::z_linear_assignment_t& stmt,
                  VariableFactory&) {
  // KILL
  inv -= stmt.lhs().name();
  // GEN
  variable_set_t vars = stmt.rhs().variables();
  for (variable_set_t::iterator it = vars.begin(); it != vars.end(); ++it)
    inv += (*it).name();
}

template <>
inline void visit(liveness_domain_t& inv,
                  visitor_t::z_linear_assertion_t& stmt,
                  VariableFactory&) {
  // GEN
  variable_set_t vars = stmt.constraint().variables();
  for (variable_set_t::iterator it = vars.begin(); it != vars.end(); ++it)
    inv += (*it).name();
}

template <>
inline void visit(liveness_domain_t& inv,
                  visitor_t::z_array_write_t& stmt,
                  VariableFactory&) {
  // GEN
  variable_set_t idx_vars = stmt.index().variables();
  for (variable_set_t::iterator it = idx_vars.begin(); it != idx_vars.end();
       ++it)
    inv += (*it).name();
  variable_set_t val_vars = stmt.val().variables();
  for (variable_set_t::iterator it = val_vars.begin(); it != val_vars.end();
       ++it)
    inv += (*it).name();
  inv += stmt.array().name();
}

template <>
inline void visit(liveness_domain_t& inv,
                  visitor_t::z_array_read_t& stmt,
                  VariableFactory&) {
  // KILL
  inv -= stmt.lhs().name();
  // GEN
  variable_set_t idx_vars = stmt.index().variables();
  for (variable_set_t::iterator it = idx_vars.begin(); it != idx_vars.end();
       ++it) {
    inv += (*it).name();
  }
  inv += stmt.array().name();
}
}; // end namespace stmt_analyzer_impl

// cpa
namespace stmt_analyzer_impl {
template <>
inline void visit(cpa_domain_t& inv,
                  visitor_t::z_binary_operation_t& stmt,
                  VariableFactory&) {
  cpa_domain_t kill = inv[stmt.lhs().name()];
  cpa_domain_t gen = cpa_domain_t::bottom();
  typedef cpa_domain_t::substitution_t subst_t;
  typedef cpa_domain_t::expression_t exp_t;
  switch (stmt.operation()) {
    case ikos::OP_ADDITION:
      gen += subst_t(stmt.lhs().name(),
                     exp_t::variable(stmt.left_operand().name()) +
                         exp_t::variable(stmt.right_operand().name()));
      break;
    case ikos::OP_SUBTRACTION:
      gen += subst_t(stmt.lhs().name(),
                     exp_t::variable(stmt.left_operand().name()) -
                         exp_t::variable(stmt.right_operand().name()));
      break;
    case ikos::OP_MULTIPLICATION:
      gen += subst_t(stmt.lhs().name(),
                     exp_t::variable(stmt.left_operand().name()) *
                         exp_t::variable(stmt.right_operand().name()));
      break;
    case ikos::OP_DIVISION:
      gen += subst_t(stmt.lhs().name(),
                     exp_t::variable(stmt.left_operand().name()) /
                         exp_t::variable(stmt.right_operand().name()));
      break;
    default:;
      ;
  }
  inv.transfer_function(kill, gen);
}
template <>
inline void visit(cpa_domain_t& inv,
                  visitor_t::z_linear_assignment_t& stmt,
                  VariableFactory&) {
  cpa_domain_t kill = inv[stmt.lhs().name()];
  cpa_domain_t gen = cpa_domain_t::bottom();
  typedef cpa_domain_t::substitution_t subst_t;
  typedef cpa_domain_t::expression_t exp_t;
  if (stmt.rhs().is_constant()) {
    gen += subst_t(stmt.lhs().name(), exp_t::number(stmt.rhs().constant()));
  } else if (boost::optional< z_var > rhs = stmt.rhs().get_variable()) {
    gen += subst_t(stmt.lhs().name(), exp_t::variable((*rhs).name()));
  } else {
    // TODO: if stmt.rhs is a more complicated than linear expression we
    // should translate it into a expression_t
    assert(false && "unreachable");
  }
  inv.transfer_function(kill, gen);
}
template <>
inline void visit(cpa_domain_t& inv,
                  visitor_t::z_linear_assertion_t& stmt,
                  VariableFactory&) {}

template <>
inline void visit(cpa_domain_t& inv,
                  visitor_t::z_array_write_t& stmt,
                  VariableFactory&) {}

template <>
inline void visit(cpa_domain_t& inv,
                  visitor_t::z_array_read_t& stmt,
                  VariableFactory&) {
  cpa_domain_t kill = inv[stmt.lhs().name()];
  cpa_domain_t gen = cpa_domain_t::bottom();
  inv.transfer_function(kill, gen);
}

}; // end namespace stmt_analyzer_impl

cfg_t loop1(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb1_t = cfg.insert_basic_block("bb1_t");
  basic_block_t& bb1_f = cfg.insert_basic_block("bb1_f");
  basic_block_t& bb2 = cfg.insert_basic_block("bb2");
  basic_block_t& ret = cfg.insert_basic_block("ret");
  z_var n1(vfac["n1"]);
  z_var i(vfac["i"]);
  z_var a(vfac["A"]);
  z_var tmp3(vfac["tmp3"]);
  z_var tmp5(vfac["tmp5"]);
  z_var tmp6(vfac["tmp6"]);
  entry >> bb1;
  bb1 >> bb1_t;
  bb1 >> bb1_f;
  bb1_t >> bb2;
  bb2 >> bb1;
  bb1_f >> ret;
  ////////
  entry.assign(n1, 1);
  entry.assign(i, 0);
  ///////
  bb1_t.assertion(i <= 9);
  bb1_f.assertion(i >= 10);
  bb2.write(a, i, 123456);
  bb2.add(i, i, n1);
  bb2.check("end_of_loop");
  ret.sub(tmp3, i, n1);
  ret.read(tmp5, a, tmp3); // initialized
  ret.read(tmp6, a, i);    // top
  ret.check("end_of_program");
  return cfg;
}

cfg_t loop2(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb1_t = cfg.insert_basic_block("bb1_t");
  basic_block_t& bb1_f = cfg.insert_basic_block("bb1_f");
  basic_block_t& bb2 = cfg.insert_basic_block("bb2");
  basic_block_t& ret = cfg.insert_basic_block("ret");
  z_var n1(vfac["n1"]);
  z_var i(vfac["i"]);
  z_var a(vfac["A"]);
  z_var tmp1(vfac["tmp1"]);
  z_var tmp2(vfac["tmp2"]);
  z_var tmp3(vfac["tmp3"]);
  z_var tmp4(vfac["tmp4"]);
  z_var tmp5(vfac["tmp5"]);
  entry >> bb1;
  bb1 >> bb1_t;
  bb1 >> bb1_f;
  bb1_t >> bb2;
  bb2 >> bb1;
  bb1_f >> ret;
  ////////
  entry.assign(n1, 1);
  entry.assign(i, 0);
  ///////
  bb1_t.assertion(i <= 9);
  bb1_f.assertion(i >= 10);
  bb2.write(a, i, 123456);
  bb2.assign(tmp1, i);
  bb2.add(tmp2, tmp1, n1);
  bb2.assign(i, tmp2);
  bb2.check("end_of_loop");
  ret.sub(tmp3, i, n1);
  ret.read(tmp4, a, tmp3); // initialized
  ret.read(tmp5, a, i);    // top
  ret.check("end_of_program");
  return cfg;
}

cfg_t loop3(VariableFactory& vfac) {
  cfg_t cfg("loop1_entry", "ret");
  basic_block_t& loop1_entry = cfg.insert_basic_block("loop1_entry");
  basic_block_t& loop1_bb1 = cfg.insert_basic_block("loop1_bb1");
  basic_block_t& loop1_bb1_t = cfg.insert_basic_block("loop1_bb1_t");
  basic_block_t& loop1_bb1_f = cfg.insert_basic_block("loop1_bb1_f");
  basic_block_t& loop1_bb2 = cfg.insert_basic_block("loop1_bb2");
  ///
  basic_block_t& loop2_entry = cfg.insert_basic_block("loop2_entry");
  basic_block_t& loop2_bb1 = cfg.insert_basic_block("loop2_bb1");
  basic_block_t& loop2_bb1_t = cfg.insert_basic_block("loop2_bb1_t");
  basic_block_t& loop2_bb1_f = cfg.insert_basic_block("loop2_bb1_f");
  basic_block_t& loop2_bb2 = cfg.insert_basic_block("loop2_bb2");
  ///
  basic_block_t& ret = cfg.insert_basic_block("ret");

  loop1_entry >> loop1_bb1;
  loop1_bb1 >> loop1_bb1_t;
  loop1_bb1 >> loop1_bb1_f;
  loop1_bb1_t >> loop1_bb2;
  loop1_bb2 >> loop1_bb1;
  loop1_bb1_f >> loop2_entry;

  loop2_entry >> loop2_bb1;
  loop2_bb1 >> loop2_bb1_t;
  loop2_bb1 >> loop2_bb1_f;
  loop2_bb1_t >> loop2_bb2;
  loop2_bb2 >> loop2_bb1;
  loop2_bb1_f >> ret;
  /////

  z_var n1(vfac["n1"]);
  z_var i(vfac["i"]);
  z_var j(vfac["j"]);
  z_var a(vfac["A"]);
  z_var b(vfac["B"]);
  z_var tmp1(vfac["tmp1"]);
  z_var tmp2(vfac["tmp2"]);
  z_var tmp3(vfac["tmp3"]);
  z_var tmp4(vfac["tmp4"]);

  loop1_entry.assign(n1, 1);
  loop1_entry.assign(i, 0);
  loop1_bb1_t.assertion(i <= 9);
  loop1_bb1_f.assertion(i >= 10);
  loop1_bb2.write(a, i, 123456);
  loop1_bb2.add(i, i, n1);
  loop1_bb2.check("end_of_loop");

  loop2_entry.assign(j, 0);
  loop2_bb1_t.assertion(j <= 9);
  loop2_bb1_f.assertion(j >= 10);
  loop2_bb2.read(tmp1, a, j);
  loop2_bb2.write(b, j, tmp1);
  loop2_bb2.add(j, j, n1);
  loop2_bb2.check("end_of_loop");

  ret.sub(tmp2, j, n1);
  ret.read(tmp3, b, tmp2); // initialized
  ret.read(tmp4, b, j);    // top
  ret.check("end_of_program");
  return cfg;
}

cfg_t loop4(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb1_t = cfg.insert_basic_block("bb1_t");
  basic_block_t& bb1_f = cfg.insert_basic_block("bb1_f");
  basic_block_t& bb2 = cfg.insert_basic_block("bb2");
  basic_block_t& ret = cfg.insert_basic_block("ret");
  z_var n1(vfac["n1"]);
  z_var i(vfac["i"]);
  z_var a(vfac["A"]);
  z_var b(vfac["B"]);
  z_var tmp3(vfac["tmp3"]);
  z_var tmp5(vfac["tmp5"]);
  z_var tmp6(vfac["tmp6"]);
  z_var x(vfac["x"]);
  entry >> bb1;
  bb1 >> bb1_t;
  bb1 >> bb1_f;
  bb1_t >> bb2;
  bb2 >> bb1;
  bb1_f >> ret;
  ////////
  entry.assign(n1, 1);
  entry.assign(i, 0);
  ///////
  bb1_t.assertion(i <= 9);
  bb1_f.assertion(i >= 10);
  bb2.write(a, i, 8);
  bb2.write(b, i, 5);
  bb2.add(i, i, n1);
  bb2.check("end_of_loop");
  ret.sub(tmp3, i, n1);
  ret.read(tmp5, a, tmp3);
  ret.read(tmp6, b, tmp3);
  ret.check("end_of_program");
  return cfg;
}

cfg_t loop5(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb1_t = cfg.insert_basic_block("bb1_t");
  basic_block_t& bb1_f = cfg.insert_basic_block("bb1_f");
  basic_block_t& bb2 = cfg.insert_basic_block("bb2");
  basic_block_t& ret = cfg.insert_basic_block("ret");
  z_var n1(vfac["n1"]);
  z_var i(vfac["i"]);
  z_var n(vfac["n"]);
  z_var a(vfac["A"]);
  z_var tmp1(vfac["tmp1"]);
  z_var tmp2(vfac["tmp2"]);
  entry >> bb1;
  bb1 >> bb1_t;
  bb1 >> bb1_f;
  bb1_t >> bb2;
  bb2 >> bb1;
  bb1_f >> ret;
  ////////
  entry.assertion(n >= 1);
  entry.assign(n1, 1);
  entry.assign(i, 0);
  ///////
  bb1_t.assertion(i <= n - 1);
  bb1_f.assertion(i >= n);
  bb2.write(a, i, 123456);
  bb2.add(i, i, n1);
  bb2.check("end_of_loop");
  ret.sub(tmp1, i, n1);
  ret.read(tmp2, a, tmp1); // initialized
  ret.check("end_of_program");
  return cfg;
}

cfg_t loop5b(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb1_t = cfg.insert_basic_block("bb1_t");
  basic_block_t& bb1_f = cfg.insert_basic_block("bb1_f");
  basic_block_t& bb2 = cfg.insert_basic_block("bb2");
  basic_block_t& ret = cfg.insert_basic_block("ret");
  z_var c1(vfac["#1"]);
  z_var c3(vfac["#3"]);
  z_var c5(vfac["#5"]);
  z_var i(vfac["i"]);
  z_var a(vfac["A"]);
  z_var tmp(vfac["tmp"]);
  z_var tmp1_offset_2(vfac["tmp1_offset_2"]);
  z_var tmp2(vfac["tmp2"]);
  z_var tmp4(vfac["tmp4"]);

  entry >> bb1;
  bb1 >> bb1_t;
  bb1 >> bb1_f;
  bb1_t >> bb2;
  bb2 >> bb1;
  bb1_f >> ret;
  ////////
  entry.assign(i, 0);
  entry.assign(c1, 1);
  entry.assign(c5, 5);
  entry.assign(c3, 3);
  ///////
  bb1_t.assertion(i <= 4);
  bb1_f.assertion(5 <= i);
  bb2.assign(tmp, i);
  bb2.mul(tmp1_offset_2, tmp, c1); // 32
  bb2.write(a, tmp1_offset_2, 123456);
  bb2.add(tmp2, i, c1);
  bb2.assign(i, tmp2);
  bb2.check("end_of_loop");
  ret.read(tmp4, a, c3); // 160
  ret.check("end_of_program");
  return cfg;
}

cfg_t loop6(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb1_t = cfg.insert_basic_block("bb1_t");
  basic_block_t& bb1_f = cfg.insert_basic_block("bb1_f");
  basic_block_t& bb2 = cfg.insert_basic_block("bb2");
  basic_block_t& ret = cfg.insert_basic_block("ret");
  z_var n1(vfac["n1"]);
  z_var i(vfac["i"]);
  z_var n(vfac["n"]);
  z_var a(vfac["A"]);
  z_var tmp1(vfac["tmp1"]);
  z_var tmp2(vfac["tmp2"]);
  z_var tmp3(vfac["tmp3"]);
  z_var tmp4(vfac["tmp4"]);
  z_var x(vfac["x"]);
  entry >> bb1;
  bb1 >> bb1_t;
  bb1 >> bb1_f;
  bb1_t >> bb2;
  bb2 >> bb1;
  bb1_f >> ret;
  ////////
  entry.assertion(n >= 2);
  entry.assign(n1, 1);
  entry.assign(i, 0);
  entry.write(a, i, 89);
  entry.assign(i, 1);
  ///////
  bb1_t.assertion(i <= n - 1);
  bb1_f.assertion(i >= n);
  ///////
  bb2.sub(tmp1, i, n1);
  bb2.read(tmp2, a, tmp1);
  bb2.write(a, i, tmp2);
  bb2.add(i, i, n1);
  bb2.check("end_of_loop");
  ///////
  ret.sub(tmp3, n, n1);
  ret.read(tmp4, a, tmp3);
  ret.check("end_of_program");
  return cfg;
}

// Initialize only even positions
// TODO: need a reduced of octagons and congruences
cfg_t loop7(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb1_t = cfg.insert_basic_block("bb1_t");
  basic_block_t& bb1_f = cfg.insert_basic_block("bb1_f");
  basic_block_t& bb2 = cfg.insert_basic_block("bb2");
  basic_block_t& ret = cfg.insert_basic_block("ret");
  z_var n1(vfac["n1"]);
  z_var n2(vfac["n2"]);
  z_var i(vfac["i"]);
  z_var i1(vfac["i1"]);
  z_var n(vfac["n"]);
  z_var a(vfac["A"]);
  z_var tmp1(vfac["tmp1"]);
  z_var tmp2(vfac["tmp2"]);
  z_var tmp3(vfac["tmp3"]);
  entry >> bb1;
  bb1 >> bb1_t;
  bb1 >> bb1_f;
  bb1_t >> bb2;
  bb2 >> bb1;
  bb1_f >> ret;
  ////////
  entry.assertion(n >= 1);
  entry.assign(n1, 1);
  entry.assign(n2, 2);
  entry.assign(i, 0);
  ///////
  bb1_t.assertion(i <= 9);
  bb1_f.assertion(i >= 10);
  bb2.write(a, i, 123456);
  // If we comment these two lines then we do only initialization of
  // even positions.
  bb2.add(i1, i, n1);
  bb2.write(a, i1, 123);
  bb2.add(i, i, n2);
  bb2.check("end_of_loop");
  ret.assign(tmp1, 6);
  ret.read(tmp2, a, tmp1); // initialized
  ret.check("end_of_program");
  return cfg;
}

// this is the program init_rand from Gange et.al paper.
cfg_t loop8(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb1_t = cfg.insert_basic_block("bb1_t");
  basic_block_t& bb1_f1 = cfg.insert_basic_block("bb1_f1");
  basic_block_t& bb1_f2 = cfg.insert_basic_block("bb1_f2");
  basic_block_t& bb1_f = cfg.insert_basic_block("bb1_f");
  basic_block_t& bb2_a = cfg.insert_basic_block("bb2a");
  basic_block_t& bb2_b = cfg.insert_basic_block("bb2b");
  basic_block_t& ret = cfg.insert_basic_block("ret");
  z_var n1(vfac["n1"]);
  z_var i1(vfac["i1"]);
  z_var i2(vfac["i2"]);
  z_var n(vfac["n"]);
  z_var a(vfac["A"]);
  z_var tmp1(vfac["tmp1"]);
  z_var tmp2(vfac["tmp2"]);
  entry >> bb1;
  bb1 >> bb1_t;
  bb1 >> bb1_f1;
  bb1 >> bb1_f2;
  bb1_f1 >> bb1_f;
  bb1_f2 >> bb1_f;
  bb1_t >> bb2_a;
  bb1_t >> bb2_b;
  bb2_a >> bb1;
  bb2_b >> bb1;
  bb1_f >> ret;
  ////////
  entry.assertion(n >= 1);
  entry.assign(n1, 1);
  entry.assign(i1, 0);
  entry.assign(i2, 0);
  ///////
  // while (i1 < n && i2 < n){
  bb1_t.assertion(i1 <= n - 1);
  bb1_t.assertion(i2 <= n - 1);
  // if (*)
  bb2_a.write(a, i1, 123456);
  bb2_a.add(i1, i1, n1);
  // else
  bb2_b.write(a, i2, 9);
  bb2_b.add(i2, i2, n1);
  // } end while
  bb1_f1.assertion(i1 >= n);
  bb1_f2.assertion(i2 >= n);
  ret.sub(tmp1, n, n1);
  ret.read(tmp2, a, tmp1); // initialized
  ret.check("end_of_program");
  return cfg;
}

cfg_t loop9(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb7 = cfg.insert_basic_block("bb7");
  basic_block_t& pre_bb7_bb = cfg.insert_basic_block("pre_bb7_bb");
  basic_block_t& pre_bb7_bb8 = cfg.insert_basic_block("pre_bb7_bb8");
  basic_block_t& bb = cfg.insert_basic_block("bb");
  basic_block_t& bb8 = cfg.insert_basic_block("bb8");
  basic_block_t& bb5 = cfg.insert_basic_block("bb5");
  basic_block_t& pre_bb5_bb6 = cfg.insert_basic_block("pre_bb5_bb6");
  basic_block_t& pre_bb5_bb1 = cfg.insert_basic_block("pre_bb5_bb1");
  basic_block_t& bb6 = cfg.insert_basic_block("bb6");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb3 = cfg.insert_basic_block("bb3");
  basic_block_t& pre_bb3_bb4 = cfg.insert_basic_block("pre_bb3_bb4");
  basic_block_t& pre_bb3_bb2 = cfg.insert_basic_block("pre_bb3_bb2");
  basic_block_t& bb2 = cfg.insert_basic_block("bb2");
  basic_block_t& bb4 = cfg.insert_basic_block("bb4");
  basic_block_t& ret = cfg.insert_basic_block("ret");

  entry >> bb7;
  bb7 >> pre_bb7_bb;
  bb7 >> pre_bb7_bb8;
  pre_bb7_bb >> bb;
  bb >> bb5;
  pre_bb7_bb8 >> bb8;
  bb8 >> ret;
  bb5 >> pre_bb5_bb6;
  bb5 >> pre_bb5_bb1;
  pre_bb5_bb6 >> bb6;
  bb6 >> bb7;
  pre_bb5_bb1 >> bb1;
  bb1 >> bb3;
  bb3 >> pre_bb3_bb4;
  bb3 >> pre_bb3_bb2;
  pre_bb3_bb4 >> bb4;
  pre_bb3_bb2 >> bb2;
  bb4 >> bb5;
  bb2 >> bb3;

  // variables
  z_var cst_1(vfac["#1"]);
  z_var cst_40000(vfac["#40000"]);
  z_var cst_4(vfac["#4"]);
  z_var cst_400(vfac["#400"]);

  z_var i(vfac["i"]);
  z_var j(vfac["j"]);
  z_var k(vfac["k"]);
  z_var x(vfac["x"]);
  z_var tmp0(vfac["tmp0"]);
  z_var tmp1(vfac["tmp1"]);
  z_var tmp2(vfac["tmp2"]);
  z_var tmp3(vfac["tmp3"]);
  z_var temp1(vfac["temp1"]);
  z_var temp2(vfac["temp2"]);
  z_var temp3(vfac["temp3"]);
  z_var temp4(vfac["temp4"]);
  z_var temp5(vfac["temp5"]);
  z_var t0(vfac["T0"]);
  z_var t1(vfac["T1"]);

  ////////
  entry.assign(cst_40000, 40000);
  entry.assign(cst_400, 400);
  entry.assign(cst_1, 1);
  entry.assign(cst_4, 4);

  entry.assign(tmp0, 5);
  entry.assign(temp1, tmp0);
  entry.assign(i, 0);
  ///////
  pre_bb7_bb.assertion(i <= 99);
  bb.assign(tmp1, temp1);
  bb.assign(temp2, tmp1);
  bb.assign(temp3, 0);
  bb.assign(j, 0);

  pre_bb5_bb6.assertion(j >= 100);
  bb6.add(temp1, temp1, cst_40000);
  bb6.add(i, i, cst_1);

  pre_bb5_bb1.assertion(j <= 99);
  bb1.assign(tmp2, temp2);
  bb1.assign(tmp3, temp3);
  bb1.assign(temp4, 0);
  bb1.assign(temp5, tmp2);
  bb1.assign(k, 0);

  pre_bb3_bb2.assertion(k <= 9999);
  bb2.assign(t0, temp4);
  bb2.assign(t1, temp5);
  bb2.add(x, t0, t1);
  bb2.add(temp4, temp4, tmp3);
  bb2.add(temp5, temp5, cst_4);
  bb2.add(k, k, cst_1);

  pre_bb3_bb4.assertion(k >= 1000);
  bb4.add(temp2, temp2, cst_400);
  bb4.add(temp3, temp3, i);
  bb4.add(j, j, cst_1);

  pre_bb7_bb8.assertion(i >= 100);
  ret.check("end_of_program");
  return cfg;
}

cfg_t loop10(VariableFactory& vfac) {
  cfg_t cfg("entry", "ret");
  basic_block_t& entry = cfg.insert_basic_block("entry");
  basic_block_t& bb = cfg.insert_basic_block("bb");
  basic_block_t& bb1 = cfg.insert_basic_block("bb1");
  basic_block_t& bb2 = cfg.insert_basic_block("bb2");
  basic_block_t& bb3 = cfg.insert_basic_block("bb4");
  basic_block_t& ret = cfg.insert_basic_block("ret");

  entry >> bb;
  bb >> bb1;
  bb >> bb2;
  bb1 >> bb3;
  bb2 >> bb3;
  bb3 >> ret;

  // variables
  z_var a(vfac["a"]);
  z_var b(vfac["b"]);
  z_var c(vfac["c"]);
  z_var d(vfac["d"]);
  z_var e(vfac["e"]);
  z_var f(vfac["f"]);
  z_var g(vfac["g"]);
  z_var h(vfac["h"]);

  entry.assign(a, b);
  bb1.assign(c, d);
  bb2.assign(e, f);
  ret.assign(g, h);
  return cfg;
}

void run(cfg_t cfg, std::string name, VariableFactory& vfac) {
  std::cout << "--- " << name << std::endl;
  std::cout << cfg << std::endl;

  /*

  std::cout << "Running interval domain" << std::endl;
  fwd_analyzer<interval_domain_t> it1(cfg, vfac);
  it1.run(interval_domain_t::top());
  std::cout << "Running octagon domain" << std::endl;
  fwd_analyzer<octagon_domain_t> it2(cfg, vfac);
  it2.run(octagon_domain_t::top());
  std::cout << "Running dbm domain" << std::endl;
  fwd_analyzer<dbm_domain_t> it3(cfg, vfac);
  it3.run(dbm_domain_t::top());

  */

  std::cout << "Running liveness domain" << std::endl;
  backward_analyzer< liveness_domain_t > it4(cfg, vfac);
  it4.run(liveness_domain_t::bottom());

  std::cout << "Running scalar uva domain" << std::endl;
  fwd_analyzer< uva_domain_t > it5(cfg, vfac);
  it5.run(uva_domain_t::top());

  std::cout << "Running scalar+array uva domain" << std::endl;
  fwd_analyzer< uva_array_domain_t > it6(cfg, vfac);
  it6.run(uva_array_domain_t::top());

  std::cout << std::endl;
}

void test1() {
  VariableFactory vfac;
  cfg_t cfg = loop1(vfac);
  run(cfg, "Program 1", vfac);
}

void test2() {
  VariableFactory vfac;
  cfg_t cfg = loop2(vfac);
  run(cfg, "Program 2", vfac);
}

void test3() {
  VariableFactory vfac;
  cfg_t cfg = loop3(vfac);
  run(cfg, "Program 3", vfac);
}

void test4() {
  VariableFactory vfac;
  cfg_t cfg = loop4(vfac);
  run(cfg, "Program 4", vfac);
}

void test5() {
  VariableFactory vfac;
  cfg_t cfg = loop5(vfac);
  run(cfg, "Program 5", vfac);
}

void test6() {
  VariableFactory vfac;
  cfg_t cfg = loop6(vfac);
  run(cfg, "Program 6", vfac);
}

void test7() {
  VariableFactory vfac;
  cfg_t cfg = loop7(vfac);
  run(cfg, "Program 7", vfac);
}

// This is the case that cannot be handled by Logozzo,Cousot, and
// Cousot (POPL'11)
void test8() {
  VariableFactory vfac;
  cfg_t cfg = loop8(vfac);
  run(cfg, "Program 8", vfac);
}

#if 0
void test9()
{
  //  this test is just for CPA
  VariableFactory vfac;
  cfg_t cfg = loop9(vfac);
  std::cout << cfg << std::endl;
  std::cout << "Running copy propagation analysis" << std::endl;
  fwd_analyzer< cpa_domain_t > it(cfg, vfac);
  it.run(cpa_domain_t::bottom());
}
#endif

void test10() {
  VariableFactory vfac;
  cfg_t cfg = loop5b(vfac);
  run(cfg, "Program 10", vfac);
}

int main(int, char**) {
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
  test7();
  test8();
  // test10 ();
  return 0;
}
