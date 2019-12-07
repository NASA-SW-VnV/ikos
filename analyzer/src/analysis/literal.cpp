/*******************************************************************************
 *
 * \file
 * \brief Implementation of LiteralFactory
 *
 * Author: Jorge A. Navas
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2019 United States Government as represented by the
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

#include <boost/thread/locks.hpp>

#include <ikos/ar/semantic/value_visitor.hpp>

#include <ikos/analyzer/analysis/literal.hpp>

namespace ikos {
namespace analyzer {

LiteralFactory::LiteralFactory(VariableFactory& vfac,
                               const ar::DataLayout& data_layout)
    : _vfac(vfac), _data_layout(data_layout) {}

LiteralFactory::~LiteralFactory() = default;

const ScalarLit& LiteralFactory::get_scalar(ar::Value* value) {
  const Literal& lit = this->get(value);

  if (lit.is_scalar()) {
    return lit.scalar();
  } else {
    throw AggregateLiteralError(lit.aggregate());
  }
}

const AggregateLit& LiteralFactory::get_aggregate(ar::Value* value) {
  const Literal& lit = this->get(value);

  if (lit.is_aggregate()) {
    return lit.aggregate();
  } else {
    throw ScalarLiteralError(lit.scalar());
  }
}

const Literal& LiteralFactory::get(ar::Value* value) {
  {
    boost::shared_lock< boost::shared_mutex > lock(this->_mutex);
    auto it = this->_map.find(value);
    if (it != this->_map.end()) {
      return it->second;
    }
  }

  Literal literal = this->create_literal(value);

  {
    boost::unique_lock< boost::shared_mutex > lock(this->_mutex);
    std::pair< Map::iterator, bool > res =
        this->_map.emplace(value, std::move(literal));
    return res.first->second;
  }
}

namespace {

/// \brief Convert a std::size_t representing a size or an offset to a
/// MachineInt
MachineInt to_machine_int(std::size_t n, const ar::DataLayout& dl) {
  return MachineInt(n, dl.pointers.bit_width, Unsigned);
}

/// \brief Convert a ZNumber representing a size or an offset to a MachineInt
MachineInt to_machine_int(const ZNumber& n, const ar::DataLayout& dl) {
  return MachineInt(n, dl.pointers.bit_width, Unsigned);
}

/// \brief Add a Literal to a AggregateLit::Fields list
class AddAggregateField : public Literal::Visitor<> {
private:
  /// \brief Output Field list
  AggregateLit::Fields& _fields;

  /// \brief Offset of the input aggregate
  MachineInt _offset;

  /// \brief Size of the input aggregate
  MachineInt _size;

public:
  /// \brief Constructor
  AddAggregateField(AggregateLit::Fields& fields,
                    MachineInt offset,
                    MachineInt size)
      : _fields(fields), _offset(std::move(offset)), _size(std::move(size)) {}

public:
  void operator()(const ScalarLit& scalar) {
    this->_fields.push_back(
        AggregateLit::Field{this->_offset, scalar, this->_size});
  }

  void operator()(const AggregateLit& aggregate) {
    if (aggregate.is_cst()) {
      for (const auto& field : aggregate.fields()) {
        this->_fields.push_back(
            AggregateLit::Field{this->_offset + field.offset,
                                field.value,
                                field.size});
      }
    } else if (aggregate.is_zero()) {
      this->_fields.push_back(
          AggregateLit::Field{this->_offset, ScalarLit::null(), this->_size});
    } else if (aggregate.is_undefined()) {
      this->_fields.push_back(AggregateLit::Field{this->_offset,
                                                  ScalarLit::undefined(),
                                                  this->_size});
    } else if (aggregate.is_var()) {
      throw LogicError(
          "literal factory: unexpected variable aggregate within a constant "
          "aggregate");
    } else {
      ikos_unreachable("unreachable");
    }
  }

}; // end class AddAggregateField

/// \brief Create a Literal from an ar::Value*
class ValueVisitor {
public:
  using ResultType = Literal;

private:
  /// \brief Variable factory
  VariableFactory& _vfac;

  /// \brief Data layout
  const ar::DataLayout& _dl;

public:
  /// \brief Constructor
  ValueVisitor(VariableFactory& vfac, const ar::DataLayout& data_layout)
      : _vfac(vfac), _dl(data_layout) {}

public:
  Literal operator()(ar::UndefinedConstant* c) {
    if (c->type()->is_scalar()) {
      return Literal(ScalarLit::undefined());
    } else if (c->type()->is_aggregate()) {
      return Literal(AggregateLit::undefined(
          to_machine_int(_dl.store_size_in_bytes(c->type()), _dl)));
    } else {
      throw LogicError("literal factory: unexpected variable type");
    }
  }

  Literal operator()(ar::IntegerConstant* c) {
    return Literal(ScalarLit::machine_int(c->value()));
  }

  Literal operator()(ar::FloatConstant* /*f*/) {
    return Literal(ScalarLit::floating_point(DummyNumber{}));
  }

  Literal operator()(ar::NullConstant* /*n*/) {
    return Literal(ScalarLit::null());
  }

  Literal operator()(ar::StructConstant* c) {
    AggregateLit::Fields fields;

    for (auto it = c->field_begin(), et = c->field_end(); it != et; ++it) {
      // Translate element
      Literal value = ar::apply_visitor(*this, it->value);

      // Add in fields
      AddAggregateField vis(fields,
                            /*offset = */ to_machine_int(it->offset, _dl),
                            /*size = */
                            to_machine_int(_dl.store_size_in_bytes(
                                               it->value->type()),
                                           _dl));
      value.apply_visitor(vis);
    }

    return Literal(
        AggregateLit::cst(fields,
                          to_machine_int(_dl.store_size_in_bytes(c->type()),
                                         _dl)));
  }

  Literal operator()(ar::ArrayConstant* c) {
    ar::Type* element_type = c->type()->element_type();
    MachineInt alloc_size =
        to_machine_int(_dl.alloc_size_in_bytes(element_type), _dl);

    AggregateLit::Fields fields;

    std::size_t n = 0;
    for (auto it = c->element_begin(), et = c->element_end(); it != et;
         ++it, ++n) {
      // Translate element
      Literal value = ar::apply_visitor(*this, *it);

      // Add in fields
      AddAggregateField vis(fields,
                            /*offset = */ alloc_size * to_machine_int(n, _dl),
                            /*size = */ alloc_size);
      value.apply_visitor(vis);
    }

    return Literal(
        AggregateLit::cst(fields,
                          to_machine_int(_dl.store_size_in_bytes(c->type()),
                                         _dl)));
  }

  Literal operator()(ar::VectorConstant* c) {
    ar::Type* element_type = c->type()->element_type();
    MachineInt store_size =
        to_machine_int(_dl.store_size_in_bytes(element_type), _dl);

    AggregateLit::Fields fields;

    std::size_t n = 0;
    for (auto it = c->element_begin(), et = c->element_end(); it != et;
         ++it, ++n) {
      // Translate element
      Literal value = ar::apply_visitor(*this, *it);

      // Add in fields
      AddAggregateField vis(fields,
                            /*offset = */ store_size * to_machine_int(n, _dl),
                            /*size = */ store_size);
      value.apply_visitor(vis);
    }

    return Literal(
        AggregateLit::cst(fields,
                          to_machine_int(_dl.store_size_in_bytes(c->type()),
                                         _dl)));
  }

  Literal operator()(ar::AggregateZeroConstant* c) {
    return Literal(AggregateLit::zero(
        to_machine_int(_dl.store_size_in_bytes(c->type()), _dl)));
  }

  Literal operator()(ar::FunctionPointerConstant* c) {
    return Literal(ScalarLit::pointer_var(this->_vfac.get_function_ptr(c)));
  }

  Literal operator()(ar::InlineAssemblyConstant* c) {
    return Literal(ScalarLit::pointer_var(this->_vfac.get_asm_ptr(c)));
  }

  Literal operator()(ar::GlobalVariable* gv) {
    return Literal(ScalarLit::pointer_var(this->_vfac.get_global(gv)));
  }

  Literal operator()(ar::LocalVariable* lv) {
    return Literal(ScalarLit::pointer_var(this->_vfac.get_local(lv)));
  }

  Literal operator()(ar::InternalVariable* iv) {
    ar::Type* type = iv->type();
    Variable* var = this->_vfac.get_internal(iv);

    if (type->is_integer()) {
      return Literal(ScalarLit::machine_int_var(var));
    } else if (type->is_float()) {
      return Literal(ScalarLit::floating_point_var(var));
    } else if (type->is_pointer()) {
      return Literal(ScalarLit::pointer_var(var));
    } else if (type->is_aggregate()) {
      return Literal(
          AggregateLit::var(var,
                            to_machine_int(_dl.store_size_in_bytes(iv->type()),
                                           _dl)));
    } else if (type->is_void()) {
      throw LogicError("literal factory: unexpected variable of type void");
    } else {
      throw LogicError("literal factory: unexpected variable type");
    }
  }

}; // end class ValueVisitor

} // end anonymous namespace

Literal LiteralFactory::create_literal(ar::Value* value) {
  ValueVisitor vis(this->_vfac, this->_data_layout);
  return ar::apply_visitor(vis, value);
}

} // end namespace analyzer
} // end namespace ikos
