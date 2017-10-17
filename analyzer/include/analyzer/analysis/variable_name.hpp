/*******************************************************************************
 *
 * variable_name types and VariableFactory used by the analyses
 *
 * Author: Clement Decoodt
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
#ifndef ANALYZER_VARIABLE_NAME_HPP
#define ANALYZER_VARIABLE_NAME_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include <boost/noncopyable.hpp>

#include <analyzer/analysis/memory_location.hpp>
#include <analyzer/ar-wrapper/wrapper.hpp>
#include <ikos/common/types.hpp>
#include <ikos/domains/pta.hpp>
#include <ikos/number/z_number.hpp>

namespace analyzer {

//! Forward declaration for variable_name
class VariableFactory;

//! Variable name structure linking the name to the AR representation
/*!
 * variable_name is an abstract class, and implementation of the VariableName
 * generic object in the core. It is an indexable object (see
 * core/include/ikos/common/types.hpp for description of what is an indexable
 * object).
 *
 * The variable_name has a 'kind', which is the type of variable it represents.
 * The 'kind' is an element of kind_t.
 *
 * A varname_t is a pointer to variable_name, and should be passed everywhere
 * instead of passing directly the variable_name. The factory that creates the
 * variable_name owns the pointer, so you don't need to free it.
 *
 */
struct variable_name : private boost::noncopyable {
  //! Kind of the variable_name
  enum kind_t {
    _begin_ar_variable_kind,
    local_variable_kind,
    global_variable_kind,
    internal_variable_kind,
    function_addr_kind,
    _end_ar_variable_kind,
    _begin_shadow_variable_kind,
    cell_variable_kind,
    summary_tmp_variable_kind,
    offset_variable_kind,
    alloc_size_variable_kind,
    param_ref_variable_kind,
    ret_ref_variable_kind,
    special_shadow_variable_kind,
    _end_shadow_variable_kind,
  };

protected:
  //! Kind of the variable_name
  kind_t _kind;

  //! Given name of the variable
  std::string _name;

  //! XXX: VariableFactory access through the variable_name
  VariableFactory* _vfac;

  //! Protected constructor, this struct is not supposed to be used directly
  variable_name(kind_t kind, VariableFactory* vf) : _kind(kind), _vfac(vf) {}

public:
  virtual ~variable_name(){};

  //! Return the kind of the object
  kind_t kind() const { return this->_kind; }

  //! XXX: Deprecated: Return the given name of the variable
  const std::string& name() const { return this->_name; }

  //! Printed name for debugging and exporting
  //  TODO: Use purely virtual str() method to display custom information
  const std::string& str() const { return this->_name; };

  //! XXX: Variable factory getter
  VariableFactory& var_factory() const { return *(this->_vfac); }
};

/*!
 * A varname_t is a pointer to variable_name, and should be passed everywhere
 * instead of passing directly the variable_name. The factory that creates the
 * variable_name owns the pointer, so you don't need to free it.
 */
typedef variable_name* varname_t;

} // end namespace analyzer

namespace ikos {

//! Index trait class specialization for variable_name*
/*!
 * The index of variable_name* is the address of the pointer, write trait writes
 * the name of the variable_name.
 */
template <>
struct index_traits< analyzer::variable_name* > {
  static ikos::index64_t index(const analyzer::variable_name* v) {
    static_assert(sizeof(ikos::index64_t) >= sizeof(std::intptr_t),
                  "ikos::index64_t must be at least the size of an intptr_t");
    return reinterpret_cast< ikos::index64_t >(v);
  }

  static void write(std::ostream& o, analyzer::variable_name* v) {
    o << v->name();
  }
};

} // end namespace ikos

namespace analyzer {

//! Specify the hash function of a variable_name*
inline std::size_t hash_value(const variable_name* v) {
  return std::hash< ikos::index64_t >()(ikos::index_traits< variable_name* >::index(v));
}

} // end namespace analyzer

namespace std {

template <>
struct hash< analyzer::variable_name* > {
  std::size_t operator()(const analyzer::variable_name* v) const {
    return analyzer::hash_value(v);
  }
};

} // end namespace std

namespace analyzer {

//! Variable name structure for local variables
struct local_variable_name : public variable_name {
protected:
  //! Local Variable AR Representation
  arbos::Local_Variable_ref _var;

public:
  //! Default constructor
  local_variable_name(arbos::Local_Variable_ref var, VariableFactory* vf)
      : variable_name(local_variable_kind, vf), _var(var) {
    this->_name = arbos::ar::getName(var);
  }

  //! Check if the v variable_name is a local_variable_kind
  static inline bool classof(const variable_name* v) {
    return v->kind() == local_variable_kind;
  }

  //! Get the Local_Variable_ref
  arbos::Local_Variable_ref local_var() const { return this->_var; }
};

//! Variable name structure for global variables
struct global_variable_name : public variable_name {
protected:
  //! Local Variable AR Representation
  arbos::Global_Variable_ref _var;

public:
  //! Default constructor
  global_variable_name(arbos::Global_Variable_ref var, VariableFactory* vf)
      : variable_name(global_variable_kind, vf), _var(var) {
    this->_name = arbos::ar::getName(var);
  }

  //! Check if the v variable_name is a local_variable_kind
  static inline bool classof(const variable_name* v) {
    return v->kind() == global_variable_kind;
  }

  //! Get the Global_Variable_ref
  arbos::Global_Variable_ref global_var() const { return this->_var; }
};

//! Variable name structure for internal variables
struct internal_variable_name : public variable_name {
protected:
  //! Local Variable AR Representation
  arbos::Internal_Variable_ref _var;

public:
  //! Default constructor
  internal_variable_name(arbos::Internal_Variable_ref var, VariableFactory* vf)
      : variable_name(internal_variable_kind, vf), _var(var) {
    this->_name = arbos::ar::getName(var);
  }

  //! Check if the v variable_name is a local_variable_kind
  static inline bool classof(const variable_name* v) {
    return v->kind() == internal_variable_kind;
  }

  //! Get the Internal_Variable_ref
  arbos::Internal_Variable_ref internal_var() const { return this->_var; }
};

//! Variable name structure for shadow cell variables
struct cell_variable_name : public variable_name {
private:
  memory_location* _base_addr;
  ikos::z_number _offset, _size;

public:
  //! Default constructor
  cell_variable_name(memory_location* base_addr,
                     ikos::z_number offset,
                     ikos::z_number size,
                     VariableFactory* vf)
      : variable_name(cell_variable_kind, vf),
        _base_addr(base_addr),
        _offset(offset),
        _size(size) {
    std::ostringstream buf;
    buf << "C{" << this->_base_addr->str() << "," << this->_offset << ","
        << this->_size << "}";
    this->_name = buf.str();
  }

  //! Check if the v variable_name is a cell_variable_kind
  static bool inline classof(const variable_name* v) {
    return v->kind() == cell_variable_kind;
  }

  //! Get the variable_name* the cell is linked to
  memory_location* base_addr() const { return this->_base_addr; }

  //! Get the offset of the cell
  ikos::z_number offset() const { return this->_offset; }

  //! Get the size of the cell
  ikos::z_number size() const { return this->_size; }
};

//! Variable name structure for shadow summary cell variables
struct sum_cell_variable_name : public variable_name {
private:
  memory_location* _base_addr;
  ikos::z_number _offset;
  ikos::bound< ikos::z_number > _size;
  bool _output;

public:
  //! Default constructor
  sum_cell_variable_name(memory_location* base_addr,
                         ikos::z_number offset,
                         ikos::bound< ikos::z_number > size,
                         bool output,
                         VariableFactory* vf)
      : variable_name(cell_variable_kind, vf),
        _base_addr(base_addr),
        _offset(offset),
        _size(size),
        _output(output) {
    std::ostringstream buf;
    buf << "C{" << this->_base_addr->str() << "," << this->_offset << ","
        << this->_size;
    buf << "," << (this->_output ? "O" : "I");
    buf << "}";
    this->_name = buf.str();
  }

  //! Check if the v variable_name is a cell_variable_kind
  static bool inline classof(const variable_name* v) {
    return v->kind() == cell_variable_kind;
  }

  //! Get the variable_name the cell is linked to
  memory_location* base_addr() const { return this->_base_addr; }

  //! Get the offset of the cell
  ikos::z_number offset() const { return this->_offset; }

  //! Get the size of the cell
  ikos::bound< ikos::z_number > size() const { return this->_size; }

  //! Get the I/O value of the cell
  bool output() const { return this->_output; }
};

//! Variable name structure for summary_domain tmp variables
struct summary_tmp_variable_name : public variable_name {
public:
  //! Default constructor
  summary_tmp_variable_name(ikos::index64_t idx, VariableFactory* vf)
      : variable_name(summary_tmp_variable_kind, vf) {
    std::ostringstream buf;
    buf << "shadow.tmp." << idx;
    this->_name = buf.str();
  }

  //! Check if the v variable_name is a summary_tmp_variable_kind
  static bool inline classof(const variable_name* v) {
    return v->kind() == summary_tmp_variable_kind;
  }
};

//! Variable name structure for offset variables
struct offset_variable_name : public variable_name {
private:
  variable_name* _pointer_vn;

public:
  //! Default constructor
  offset_variable_name(variable_name* pointer_vn, VariableFactory* vf)
      : variable_name(offset_variable_kind, vf), _pointer_vn(pointer_vn) {
    std::ostringstream buf;
    ikos::index_traits< variable_name* >::write(buf, _pointer_vn);
    buf << ".offset";
    this->_name = buf.str();
  }

  //! Check if the v variable_name is a summary_tmp_variable_kind
  static bool inline classof(const variable_name* v) {
    return v->kind() == offset_variable_kind;
  }

  //! Get the pointer variable name
  variable_name* pointer() const { return this->_pointer_vn; }
};

//! Variable name structure for alloc_size_variable
struct alloc_size_variable_name : public variable_name {
private:
  //! Base address
  memory_location* _base_addr;

public:
  //! Default constructor
  alloc_size_variable_name(memory_location* base_addr, VariableFactory* vf)
      : variable_name(offset_variable_kind, vf), _base_addr(base_addr) {
    std::ostringstream buf;
    buf << "shadow." << _base_addr->str() << ".size";
    this->_name = buf.str();
  }

  //! Check if the v variable_name is a alloc_size_variable_kind
  static bool inline classof(const variable_name* v) {
    return v->kind() == alloc_size_variable_kind;
  }

  //! Get the base address
  memory_location* base_addr() const { return this->_base_addr; }
};

//! Variable name structure for ikos param_ref variable
struct param_ref_variable_name : public variable_name {
private:
  ikos::param_ref _param_ref;

public:
  //! Default constructor
  param_ref_variable_name(ikos::param_ref param_ref, VariableFactory* vf)
      : variable_name(param_ref_variable_kind, vf), _param_ref(param_ref) {
    std::ostringstream buf;
    this->_param_ref.print(buf);
    this->_name = buf.str();
  }

  //! Check if the v variable_name is a param_ref_variable_kind
  static bool inline classof(const variable_name* v) {
    return v->kind() == param_ref_variable_kind;
  }

  //! Get the ikos param_ref
  ikos::param_ref param_ref() const { return this->_param_ref; }
};

//! Variable name structure for ikos ret_ref variable
struct ret_ref_variable_name : public variable_name {
private:
  ikos::return_ref _ret_ref;

public:
  //! Default constructor
  ret_ref_variable_name(ikos::return_ref ret_ref, VariableFactory* vf)
      : variable_name(ret_ref_variable_kind, vf), _ret_ref(ret_ref) {
    std::ostringstream buf;
    this->_ret_ref.print(buf);
    this->_name = buf.str();
  }

  //! Check if the v variable_name is a param_ref_variable_kind
  static bool inline classof(const variable_name* v) {
    return v->kind() == ret_ref_variable_kind;
  }

  //! Get the ikos return ref
  ikos::return_ref return_ref() const { return this->_ret_ref; }
};

//! Variable name structure for special shadow variable
//  These are special variable names that have by definition a unique (between
//  shadow_variable_name) name, and are totally unrelated to any variable_name
//  or AR node.
struct special_shadow_variable_name : public variable_name {
public:
  //! Default constructor
  special_shadow_variable_name(const std::string& name, VariableFactory* vf)
      : variable_name(special_shadow_variable_kind, vf) {
    this->_name = name;
  }

  //! Check if the v variable_name is a special_shadow_variable_kind
  static bool inline classof(const variable_name* v) {
    return v->kind() == special_shadow_variable_kind;
  }
};

//! Variable name structure for function address variables
struct function_addr_variable_name : public variable_name {
private:
  arbos::Function_Type_ref _var_type;

public:
  //! Default constructor
  function_addr_variable_name(arbos::Function_Type_ref var_type,
                              const std::string& name,
                              VariableFactory* vf)
      : variable_name(function_addr_kind, vf), _var_type(var_type) {
    this->_name = name;
  }

  //! Check if the v variable_name is a shadow_variable_kind
  static bool inline classof(const variable_name* v) {
    return v->kind() == function_addr_kind;
  }

  //! Get the variable type
  arbos::Function_Type_ref var_type() const { return this->_var_type; }
};

//! Simple management for variable names
class VariableFactory : public boost::noncopyable {
private:
  template < typename Key, typename Value >
  using map_t = std::unordered_map< Key, std::unique_ptr< Value > >;

  map_t< std::string, global_variable_name > _global_variable_map;
  map_t< std::string, local_variable_name > _local_variable_map;
  map_t< std::string, internal_variable_name > _internal_variable_map;
  map_t< std::string, function_addr_variable_name > _function_addr_map;
  std::vector< std::unique_ptr< summary_tmp_variable_name > >
      _summary_tmp_vector;
  map_t< variable_name*, offset_variable_name > _offset_map;
  map_t< memory_location*, alloc_size_variable_name > _alloc_size_map;

  struct _cell_map_key_hash {
    std::size_t operator()(
        const std::tuple< memory_location*, ikos::z_number, ikos::z_number >& k)
        const {
      std::size_t key_hash = 0;
      boost::hash_combine(key_hash,
                          std::hash< memory_location* >{}(std::get< 0 >(k)));
      boost::hash_combine(key_hash,
                          std::hash< ikos::z_number >{}(std::get< 1 >(k)));
      boost::hash_combine(key_hash,
                          std::hash< ikos::z_number >{}(std::get< 2 >(k)));
      return key_hash;
    }
  };
  std::unordered_map<
      std::tuple< memory_location*, ikos::z_number, ikos::z_number >,
      std::unique_ptr< cell_variable_name >,
      _cell_map_key_hash >
      _cell_map;

  struct _sum_cell_map_key_hash {
    std::size_t operator()(const std::tuple< memory_location*,
                                             ikos::z_number,
                                             ikos::bound< ikos::z_number >,
                                             bool >& k) const {
      std::size_t key_hash = 0;
      boost::hash_combine(key_hash,
                          std::hash< memory_location* >{}(std::get< 0 >(k)));
      boost::hash_combine(key_hash,
                          std::hash< ikos::z_number >{}(std::get< 1 >(k)));
      std::ostringstream buf;
      buf << std::get< 2 >(k);
      boost::hash_combine(key_hash, std::hash< std::string >{}(buf.str()));
      boost::hash_combine(key_hash, std::hash< bool >{}(std::get< 3 >(k)));
      return key_hash;
    };
  };
  std::unordered_map< std::tuple< memory_location*,
                                  ikos::z_number,
                                  ikos::bound< ikos::z_number >,
                                  bool >,
                      std::unique_ptr< sum_cell_variable_name >,
                      _sum_cell_map_key_hash >
      _sum_cell_map;
  map_t< std::string, param_ref_variable_name > _param_ref_map;
  map_t< std::string, ret_ref_variable_name > _return_ref_map;
  map_t< std::string, special_shadow_variable_name > _special_shadow_map;

  //! Summary tmp variable name counter
  ikos::index64_t _summary_tmp_index = 1;

public:
  //! VariableName type
  typedef variable_name* variable_name_t;

public:
  //! Default constructor for factory
  VariableFactory() {}

  //! Get or Create a global_variable_name
  global_variable_name* get_global(arbos::Global_Variable_ref gv) {
    auto it = _global_variable_map.find(arbos::ar::getName(gv));
    if (it == _global_variable_map.end()) {
      global_variable_name* vn = new global_variable_name(gv, this);
      _global_variable_map.emplace(arbos::ar::getName(gv),
                                   std::unique_ptr< global_variable_name >(vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create a local_variable_name
  local_variable_name* get_local(arbos::Local_Variable_ref lv) {
    auto it = _local_variable_map.find(arbos::ar::getName(lv));
    if (it == _local_variable_map.end()) {
      local_variable_name* vn = new local_variable_name(lv, this);
      _local_variable_map.emplace(arbos::ar::getName(lv),
                                  std::unique_ptr< local_variable_name >(vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create an internal_variable_name
  internal_variable_name* get_internal(arbos::Internal_Variable_ref iv) {
    auto it = _internal_variable_map.find(arbos::ar::getName(iv));
    if (it == _internal_variable_map.end()) {
      internal_variable_name* vn = new internal_variable_name(iv, this);
      _internal_variable_map.emplace(arbos::ar::getName(iv),
                                     std::unique_ptr< internal_variable_name >(
                                         vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create a function_addr_variable_name
  function_addr_variable_name* get_function_addr(
      arbos::Function_Addr_Constant_ref fav) {
    auto it = _function_addr_map.find(fav->getFunctionName());
    if (it == _function_addr_map.end()) {
      arbos::Type_ref type_ref = fav->getType();
      // Assert the fct_type is really a function_addr_constant
      assert(arbos::ar::ar_internal::is_function_type(type_ref));
      arbos::Function_Type_ref fct_type =
          arbos::node_cast< arbos::AR_Function_Type >(type_ref);

      function_addr_variable_name* vn =
          new function_addr_variable_name(fct_type,
                                          fav->getFunctionName(),
                                          this);
      _function_addr_map.emplace(fav->getFunctionName(),
                                 std::unique_ptr< function_addr_variable_name >(
                                     vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create a function_addr_variable_name
  function_addr_variable_name* get_function_addr(arbos::Function_ref fav) {
    auto it = _function_addr_map.find(fav->getFunctionId());
    if (it == _function_addr_map.end()) {
      function_addr_variable_name* vn =
          new function_addr_variable_name(fav->getFunctionType(),
                                          fav->getFunctionId(),
                                          this);
      _function_addr_map.emplace(fav->getFunctionId(),
                                 std::unique_ptr< function_addr_variable_name >(
                                     vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create a cell_variable_name
  cell_variable_name* get_cell(memory_location* base_addr,
                               ikos::z_number offset,
                               ikos::z_number size) {
    auto key = std::make_tuple(base_addr, offset, size);
    auto it = _cell_map.find(key);
    if (it == _cell_map.end()) {
      cell_variable_name* vn =
          new cell_variable_name(base_addr, offset, size, this);
      _cell_map.emplace(key, std::unique_ptr< cell_variable_name >(vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create a sum_cell_variable_name
  sum_cell_variable_name* get_sum_cell(memory_location* base_addr,
                                       ikos::z_number offset,
                                       ikos::bound< ikos::z_number > size,
                                       bool output) {
    auto key = std::make_tuple(base_addr, offset, size, output);
    auto it = _sum_cell_map.find(key);
    if (it == _sum_cell_map.end()) {
      sum_cell_variable_name* vn =
          new sum_cell_variable_name(base_addr, offset, size, output, this);
      _sum_cell_map.emplace(key, std::unique_ptr< sum_cell_variable_name >(vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Create a summary_tmp_variable_name (Always create, never lookup)
  summary_tmp_variable_name* get_summary_tmp() {
    summary_tmp_variable_name* vn =
        new summary_tmp_variable_name(this->_summary_tmp_index, this);
    this->_summary_tmp_index++;
    _summary_tmp_vector.push_back(
        std::unique_ptr< summary_tmp_variable_name >(vn));
    return vn;
  }

  //! Get or Create an offset_variable_name
  offset_variable_name* get_offset(variable_name* pointer_vn) {
    auto it = _offset_map.find(pointer_vn);
    if (it == _offset_map.end()) {
      offset_variable_name* vn = new offset_variable_name(pointer_vn, this);
      _offset_map.emplace(pointer_vn,
                          std::unique_ptr< offset_variable_name >(vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create an alloc_size_variable_name
  alloc_size_variable_name* get_alloc_size(memory_location* base_addr) {
    auto it = _alloc_size_map.find(base_addr);
    if (it == _alloc_size_map.end()) {
      alloc_size_variable_name* vn =
          new alloc_size_variable_name(base_addr, this);
      _alloc_size_map.emplace(base_addr,
                              std::unique_ptr< alloc_size_variable_name >(vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create a param_ref_variable_name
  param_ref_variable_name* get_param(ikos::param_ref param_ref) {
    std::ostringstream buf;
    param_ref.print(buf);
    std::string name = buf.str();
    auto it = _param_ref_map.find(name);
    if (it == _param_ref_map.end()) {
      param_ref_variable_name* vn =
          new param_ref_variable_name(param_ref, this);
      _param_ref_map.emplace(name,
                             std::unique_ptr< param_ref_variable_name >(vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! create a ret_ref_variable_name
  ret_ref_variable_name* get_ret(ikos::return_ref ret_ref) {
    std::ostringstream buf;
    ret_ref.print(buf);
    std::string name = buf.str();
    auto it = _return_ref_map.find(name);
    if (it == _return_ref_map.end()) {
      ret_ref_variable_name* vn = new ret_ref_variable_name(ret_ref, this);
      _return_ref_map.emplace(name,
                              std::unique_ptr< ret_ref_variable_name >(vn));
      return vn;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create a special_shadow_variable_name
  special_shadow_variable_name* get_special_shadow(const std::string& name) {
    auto it = _special_shadow_map.find(name);
    if (it == _special_shadow_map.end()) {
      special_shadow_variable_name* vn =
          new special_shadow_variable_name(name, this);
      _special_shadow_map
          .emplace(name, std::unique_ptr< special_shadow_variable_name >(vn));
      return vn;
    } else {
      return it->second.get();
    }
  }
}; // class VariableFactory

} // end namespace analyzer

#endif // ANALYZER_VARIABLE_NAME_HPP
