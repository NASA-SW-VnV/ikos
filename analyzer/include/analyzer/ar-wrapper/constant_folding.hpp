/*******************************************************************************
 *
 * Constant Folding for ARBOS IR
 *
 * Authors: Jorge A. Navas
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2017 United States Government as represented by the
 * Administrator of the National Aeronautics and Space Administration.
 * All Rights Reserved.
 *
 * Disclaimers:
 *
 * No Warranty: THE SUBJECT SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY OF
 * ANY KIND, EITHER EXPRESSED, IMPLIED, OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL CONFORM TO SPECIFICATIONS,
 * ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE,
 * OR FREEDOM FROM INFRINGEMENT, ANY WARRANTY THAT THE SUBJECT SOFTWARE WILL BE
 * ERROR FREE, OR ANY WARRANTY THAT DOCUMENTATION, IF PROVIDED, WILL CONFORM TO
 * THE SUBJECT SOFTWARE. THIS AGREEMENT DOES NOT, IN ANY MANNER, CONSTITUTE AN
 * ENDORSEMENT BY GOVERNMENT AGENCY OR ANY PRIOR RECIPIENT OF ANY RESULTS,
 * RESULTING DESIGNS, HARDWARE, SOFTWARE PRODUCTS OR ANY OTHER APPLICATIONS
 * RESULTING FROM USE OF THE SUBJECT SOFTWARE.  FURTHER, GOVERNMENT AGENCY
 * DISCLAIMS ALL WARRANTIES AND LIABILITIES REGARDING THIRD-PARTY SOFTWARE,
 * IF PRESENT IN THE ORIGINAL SOFTWARE, AND DISTRIBUTES IT "AS IS."
 *
 * Waiver and Indemnity:  RECIPIENT AGREES TO WAIVE ANY AND ALL CLAIMS AGAINST
 * THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS, AS WELL
 * AS ANY PRIOR RECIPIENT.  IF RECIPIENT'S USE OF THE SUBJECT SOFTWARE RESULTS
 * IN ANY LIABILITIES, DEMANDS, DAMAGES, EXPENSES OR LOSSES ARISING FROM SUCH
 * USE, INCLUDING ANY DAMAGES FROM PRODUCTS BASED ON, OR RESULTING FROM,
 * RECIPIENT'S USE OF THE SUBJECT SOFTWARE, RECIPIENT SHALL INDEMNIFY AND HOLD
 * HARMLESS THE UNITED STATES GOVERNMENT, ITS CONTRACTORS AND SUBCONTRACTORS,
 * AS WELL AS ANY PRIOR RECIPIENT, TO THE EXTENT PERMITTED BY LAW.
 * RECIPIENT'S SOLE REMEDY FOR ANY SUCH MATTER SHALL BE THE IMMEDIATE,
 * UNILATERAL TERMINATION OF THIS AGREEMENT.
 *
 ******************************************************************************/

#ifndef ANALYZER_CONSTANT_FOLDING_HPP
#define ANALYZER_CONSTANT_FOLDING_HPP

#include <analyzer/ar-wrapper/transformations.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>

namespace arbos {
namespace transformations {

class ConstantFolding {
  typedef stmt_replace_operands< Operand_ref > stmt_replace_t;
  typedef stmt_replace_t::substitution_t subst_t;

  typedef std::unordered_map< Operand_ref,
                              Operand_ref,
                              Operand_Hasher::hash,
                              Operand_Hasher::eq > subst_map_t;

  subst_map_t _map;

  struct mkBind
      : public std::unary_function< Statement_ref,
                                    std::pair< Statement_ref, bool > > {
    std::pair< Statement_ref, bool > operator()(Statement_ref s) {
      return std::make_pair(s, false);
    }
  };

  void apply_substitution_map(Statement_ref stmt, Operand_ref op) {
    subst_map_t::iterator r_it = _map.find(op);
    if (r_it != _map.end()) {
      stmt_replace_t::stmt_replace_ptr visitor(
          new stmt_replace_t(subst_t(r_it->first, r_it->second)));
      ar::accept(stmt, visitor);
    }
  }

  bool build_substitution_map(ArithOp op,
                              Internal_Variable_ref res,
                              Operand_ref left,
                              Operand_ref right) {
    bool remove = false;
    switch (op) {
      case add: {
        boost::optional< z_number > x = getIntCst(remap(left));
        boost::optional< z_number > y = getIntCst(remap(right));
        if (x && y) {
          Operand_ref subst_left = Null_ref;
          convert< Internal_Variable_ref, Operand_ref >(res, subst_left);
          Operand_ref subst_right = makeIntCst(*x + *y, ar::getType(res));
          subst_map_t::iterator m_it = _map.find(subst_left);
          if (m_it != _map.end()) {
            m_it->second = subst_right;
          } else {
            _map.insert(std::make_pair(subst_left, subst_right));
          }
          remove = true;
        } else if (x && (*x == 0)) {
          Operand_ref subst_left = Null_ref;
          convert< Internal_Variable_ref, Operand_ref >(res, subst_left);
          subst_map_t::iterator m_it = _map.find(subst_left);
          if (m_it != _map.end()) {
            m_it->second = remap(right);
          } else {
            _map.insert(std::make_pair(subst_left, remap(right)));
          }
          remove = true;
        } else if (y && (*y == 0)) {
          Operand_ref subst_left = Null_ref;
          convert< Internal_Variable_ref, Operand_ref >(res, subst_left);
          subst_map_t::iterator m_it = _map.find(subst_left);
          if (m_it != _map.end()) {
            m_it->second = remap(left);
          } else {
            _map.insert(std::make_pair(subst_left, remap(left)));
          }
          remove = true;
        } else {
          Operand_ref subst_left = Null_ref;
          convert< Internal_Variable_ref, Operand_ref >(res, subst_left);
          subst_map_t::iterator m_it = _map.find(subst_left);
          if (m_it != _map.end()) {
            _map.erase(m_it);
          }
        }
      } break;
      case mul: {
        boost::optional< z_number > x = getIntCst(remap(left));
        boost::optional< z_number > y = getIntCst(remap(right));
        if (x && y) {
          Operand_ref subst_left = Null_ref;
          convert< Internal_Variable_ref, Operand_ref >(res, subst_left);
          Operand_ref subst_right = makeIntCst((*x) * (*y), ar::getType(res));
          subst_map_t::iterator m_it = _map.find(subst_left);
          if (m_it != _map.end()) {
            m_it->second = subst_right;
          } else {
            _map.insert(std::make_pair(subst_left, subst_right));
          }
          remove = true;
        } else if (x && (*x == 0)) {
          Operand_ref subst_left = Null_ref;
          convert< Internal_Variable_ref, Operand_ref >(res, subst_left);
          subst_map_t::iterator m_it = _map.find(subst_left);
          if (m_it != _map.end()) {
            m_it->second = left;
          } else {
            _map.insert(
                std::make_pair(subst_left, makeIntCst(0, ar::getType(res))));
          }
          remove = true;
        } else if (y && (*y == 0)) {
          Operand_ref subst_right = Null_ref;
          convert< Internal_Variable_ref, Operand_ref >(res, subst_right);
          subst_map_t::iterator m_it = this->_map.find(subst_right);
          if (m_it != this->_map.end()) {
            m_it->second = right;
          } else {
            _map.insert(
                std::make_pair(subst_right, makeIntCst(0, ar::getType(res))));
          }
          remove = true;
        } else {
          Operand_ref subst_left = Null_ref;
          convert< Internal_Variable_ref, Operand_ref >(res, subst_left);
          subst_map_t::iterator m_it = _map.find(subst_left);
          if (m_it != _map.end()) {
            _map.erase(m_it);
          }
        }
      } break;
      default: {
        Operand_ref subst_left = Null_ref;
        convert< Internal_Variable_ref, Operand_ref >(res, subst_left);
        subst_map_t::iterator m_it = _map.find(subst_left);
        if (m_it != _map.end()) {
          _map.erase(m_it);
        }
      }
    }
    return remove;
  }

public:
  typedef std::pair< Statement_ref, bool /* true if dead code */ > binding_t;

  ConstantFolding() {}

  /****
   * POST Apply (in-place) rewritting rules such as:
   *   s: x = y * 0   -> propagate foward x=0 and remove s
   *   s: x = 0 * y   -> propagate foward x=0 and remove s
   *   s: x = 0 + y   -> propagate foward x=y and remove s
   *   s: x = y + 0   -> propagate foward x=y and remove s
   *   s: x = k1 + k2 -> where k1,k2 are constants then propagate forward
   *                     x = [[ k1+k2 ]] and remove s
   * POST: size(stmts) = size(new_stmts)
   ****/

  Operand_ref remap(const Operand_ref& o) {
    subst_map_t::iterator it = _map.find(o);
    if (it != _map.end()) {
      assert(ar::getType(o) == ar::getType(it->second) &&
             "Cannot replace operands with different types");
      return it->second;
    } else {
      return o;
    }
  }

  template < typename Range >
  inline void apply(
      Range stmts, std::vector< std::pair< Statement_ref, bool > >& new_stmts) {
    if (stmts.empty())
      return;

    new_stmts.clear();
    std::transform(stmts.begin(),
                   stmts.end(),
                   back_inserter(new_stmts),
                   mkBind());
    typedef std::vector< std::pair< Statement_ref, bool > >::iterator iterator;
    bool changed = true;
    // There is not need of having a loop until no change since it is
    // enough to traverse forward the statements just once.
    while (changed) {
      changed = false;
      unsigned int i = 0;
      for (iterator it = new_stmts.begin(), et = new_stmts.end(); it != et;
           ++it, ++i) {
        // already marked as dead
        if ((*it).second)
          continue;

        if (ar::is_conv_stmt((*it).first)) {
          // apply the substitution map to the statement's rhs
          Conv_Op_ref s = node_cast< Conv_Op >((*it).first);
          apply_substitution_map((*it).first, ar::getRightOp(s));
        } else if (ar::is_int_arith_stmt((*it).first)) {
          // apply the substitution map to the statement's rhs
          Arith_Op_ref s = node_cast< Arith_Op >((*it).first);
          apply_substitution_map((*it).first, ar::getLeftOp(s));
          apply_substitution_map((*it).first, ar::getRightOp(s));
          // add new substitutions
          new_stmts[i].second = build_substitution_map(ar::getArithOp(s),
                                                       ar::getResult(s),
                                                       ar::getLeftOp(s),
                                                       ar::getRightOp(s));
        } else if (ar::is_pointer_shift_stmt((*it).first)) {
          // Operand_ref's cannot be compared directly using their
          // UID's.  Instead, we need to use Operand_Hasher class
          arbos::Operand_Hasher::hash op_h;
          arbos::Internal_Variable_Hasher::hash iv_h;

          // apply the substitution map to the statement's rhs
          Pointer_Shift_ref s = node_cast< Pointer_Shift >((*it).first);
          bool is_recursive = (iv_h(ar::getResult(s)) == op_h(ar::getBase(s)));
          apply_substitution_map((*it).first, ar::getBase(s));
          apply_substitution_map((*it).first, ar::getOffset(s));
          // add new substitutions
          bool remove = build_substitution_map(add,
                                               ar::getResult(s),
                                               ar::getBase(s),
                                               ar::getOffset(s));
          new_stmts[i].second = !is_recursive && remove;
        }
      } // end for
    }   // end while
  }

  void write(std::ostream& o) {
    o << "{";
    for (subst_map_t::iterator it = this->_map.begin();
         it != this->_map.end();) {
      o << it->first;
      o << " -> ";
      o << it->second;
      ++it;
      if (it != this->_map.end())
        o << "; ";
    }
    o << "}";
  }

}; // end class ConstantFolding

} // end namespace transformations
} // end namespace arbos

#endif // ANALYZER_CONSTANT_FOLDING_HPP
