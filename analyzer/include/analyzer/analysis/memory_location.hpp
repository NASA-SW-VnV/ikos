/*******************************************************************************
 *
 * memory_location types and memory_factory used by the analyses
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
#ifndef ANALYZER_MEMORY_LOCATION_HPP
#define ANALYZER_MEMORY_LOCATION_HPP

#include <memory>
#include <string>
#include <unordered_map>

#include <analyzer/ar-wrapper/wrapper.hpp>
#include <ikos/common/types.hpp>
#include <ikos/number/z_number.hpp>

namespace analyzer {

//! Memory Location structure representing a location in memory
/*!
 * memory_location is an implementation of the MemoryLocation generic object in
 * the core. It is an indexable object (see core/include/ikos/common/types.hpp
 * for further description).
 */
struct memory_location : private boost::noncopyable {
  //! kind of the memory_location
  enum kind_t {
    local_memory_kind,
    global_memory_kind,
    function_memory_kind,
    aggregate_memory_kind,
    va_arg_memory_kind,
    fixed_address_memory_kind,
    dyn_alloc_memory_kind,
  };

protected:
  //! Kind of the memory_location
  kind_t _kind;

  //! Protected constructor, this struct is not supposed to be used directly
  memory_location(kind_t kind) : _kind(kind) {}

public:
  virtual ~memory_location(){};

  //! Return the kind of the object
  kind_t kind() const { return this->_kind; }

  //! Printed name for debugging and exporting
  virtual std::string str() const = 0;
};

/*!
 * A memloc_t is a pointer to memory_location, and should be passed everywhere
 * instead of passing directly the memory_location. The memory_factory that
 * creates the memory_location owns the pointer, so you don't need to free it.
 */
typedef memory_location* memloc_t;

//! Local memory location
struct local_memory_location : public memory_location {
private:
  //! Local Variable base
  arbos::Local_Variable_ref _lv;

public:
  //! Default constructor
  local_memory_location(arbos::Local_Variable_ref lv)
      : memory_location(local_memory_kind), _lv(lv) {}

  //! Check if the ml memory_location is a local_memory_location
  static inline bool classof(const memory_location* ml) {
    return ml->kind() == local_memory_kind;
  }

  //! Printed name for debugging and exporting
  std::string str() const { return arbos::ar::getName(_lv); };

  //! Local_Variable_ref getter
  arbos::Local_Variable_ref local_variable() const { return _lv; }
};

//! Global memory location
struct global_memory_location : public memory_location {
private:
  //! Global Variable base
  arbos::Global_Variable_ref _gv;

public:
  //! Default constructor
  global_memory_location(arbos::Global_Variable_ref gv)
      : memory_location(global_memory_kind), _gv(gv) {}

  //! Check if the ml memory_location is a global_memory_location
  static inline bool classof(const memory_location* ml) {
    return ml->kind() == global_memory_kind;
  }
  //! Printed name for debugging and exporting
  std::string str() const { return arbos::ar::getName(_gv); };

  //! Global Variable getter
  arbos::Global_Variable_ref global_variable() const { return _gv; }
};

//! Function memory location
struct function_memory_location : public memory_location {
private:
  //! Function base return type
  arbos::Function_Type_ref _var_type;

  //! Function name
  std::string _name;

public:
  //! Default constructor
  function_memory_location(arbos::Function_Type_ref var_type,
                           const std::string& name)
      : memory_location(function_memory_kind),
        _var_type(var_type),
        _name(name) {}

  //! Check if the ml memory_location is a global_memory_location
  static inline bool classof(const memory_location* ml) {
    return ml->kind() == function_memory_kind;
  }

  //! Printed name for debugging and exporting
  std::string str() const { return _name; };

  //! Get the function name
  const std::string& name() { return _name; }
};

//! Aggregate memory location
struct aggregate_memory_location : public memory_location {
private:
  //! Internal Variable Name
  arbos::Internal_Variable_ref _iv;

public:
  //! Default constructor
  aggregate_memory_location(arbos::Internal_Variable_ref iv)
      : memory_location(aggregate_memory_kind), _iv(iv) {}

  //! Check if the ml memory_location is an aggregate_memory_location
  static inline bool classof(const memory_location* ml) {
    return ml->kind() == aggregate_memory_kind;
  }

  //! Printed name for debugging and exporting
  std::string str() const { return arbos::ar::getName(_iv); };

  //! Internal variable getter
  arbos::Internal_Variable_ref internal_variable() const { return _iv; }
};

//! VA Arg memory location
struct va_arg_memory_location : public memory_location {
private:
  //! Shadow Variable string
  std::string _sv;

public:
  //! Default constructor
  va_arg_memory_location(const std::string& sv)
      : memory_location(va_arg_memory_kind), _sv(sv) {}

  //! Check if the ml memory_location is a va_arg_memory_kind
  static inline bool classof(const memory_location* ml) {
    return ml->kind() == va_arg_memory_kind;
  }

  //! Printed name for debugging and exporting
  std::string str() const { return _sv; };

  //! Shadow variable getter
  const std::string& special_shadow() const { return _sv; }
};

//! Fixed address memory location
struct fixed_address_memory_location : public memory_location {
private:
  //! Address of the memory location
  ikos::z_number _addr;

public:
  //! Default constructor
  fixed_address_memory_location(ikos::z_number addr)
      : memory_location(fixed_address_memory_kind), _addr(addr) {}

  //! Check if the ml memory_location is a fixed_address_memory_kind
  static inline bool classof(const memory_location* ml) {
    return ml->kind() == fixed_address_memory_kind;
  }

  //! Printed name for debugging and exporting
  std::string str() const { return "0x" + _addr.str(16); };

  //! Address getter
  ikos::z_number addr() const { return _addr; }
};

//! dynamic alloc memory location
struct dyn_alloc_memory_location : public memory_location {
private:
  //! Call statement of the memory location
  arbos::Call_ref _call_stmt;

public:
  //! Default constructor
  dyn_alloc_memory_location(arbos::Call_ref call_stmt)
      : memory_location(dyn_alloc_memory_kind), _call_stmt(call_stmt) {}

  //! Check if the ml memory_location is a fixed_address_memory_kind
  static inline bool classof(const memory_location* ml) {
    return ml->kind() == dyn_alloc_memory_kind;
  }

  //! Printed name for debugging and exporting
  std::string str() const {
    auto srcLoc = _call_stmt->getSourceLocation();
    auto fn = _call_stmt->getContainingBasicBlock()
                  ->getContainingCode()
                  ->getContainingFunction()
                  ->getFunctionId();
    std::ostringstream buf;
    buf << "dyn_alloc:" << fn << ":" << srcLoc->getLineNumber() << ":"
        << srcLoc->getColumnNumber();

    return buf.str();
  }

  //! Call statement getter
  arbos::Call_ref call() const { return _call_stmt; }
};

//! Simple management for memory location
class memory_factory : public boost::noncopyable {
private:
  template < typename Key, typename Value >
  using map_t = std::unordered_map< Key, std::unique_ptr< Value > >;

  map_t< std::string, local_memory_location > _local_memory_map;
  map_t< std::string, global_memory_location > _global_memory_map;
  map_t< std::string, function_memory_location> _function_memory_map;
  map_t< std::string, aggregate_memory_location > _aggregate_memory_map;
  map_t< std::string, va_arg_memory_location > _va_arg_memory_map;
  map_t< ikos::z_number, fixed_address_memory_location >
      _fixed_address_memory_map;
  map_t< arbos::Call_ref, dyn_alloc_memory_location>
      _dyn_alloc_memory_map;

public:
  //! Default constructor for factory
  memory_factory() {}

  //! Get or create a local_memory_location
  local_memory_location* get_local(arbos::Local_Variable_ref lv) {
    auto it =_local_memory_map.find(arbos::ar::getName(lv));
    if (it ==_local_memory_map.end()) {
      local_memory_location* ml = new local_memory_location(lv);
      _local_memory_map.emplace(arbos::ar::getName(lv),
                                  std::unique_ptr< local_memory_location>(ml));
      return ml;
      } else {
        return it->second.get();
      }
  }

  //! Get or create a global_memory_location
  global_memory_location* get_global(arbos::Global_Variable_ref gv) {
    auto it = _global_memory_map.find(arbos::ar::getName(gv));
    if (it == _global_memory_map.end()) {
      global_memory_location* ml = new global_memory_location(gv);
      _global_memory_map.emplace(arbos::ar::getName(gv),
                                  std::unique_ptr< global_memory_location >(ml));
      return ml;
      } else {
        return it->second.get();
      }
  }

  //! Get or Create a function_memory_location
  function_memory_location* get_function(
      arbos::Function_Addr_Constant_ref fav) {
    auto it = _function_memory_map.find(fav->getFunctionName());
    if (it == _function_memory_map.end()) {
      arbos::Type_ref type_ref = fav->getType();
      // Assert the fct_type is really a function_addr_constant
      assert(arbos::ar::ar_internal::is_function_type(type_ref));
      arbos::Function_Type_ref fct_type =
          arbos::node_cast< arbos::AR_Function_Type >(type_ref);

      function_memory_location* ml =
          new function_memory_location(fct_type, fav->getFunctionName());
      _function_memory_map.emplace(fav->getFunctionName(),
                                 std::unique_ptr< function_memory_location >(
                                     ml));
      return ml;
    } else {
      return it->second.get();
    }
  }

  //! Get or Create a function_memory_location
  function_memory_location* get_function(arbos::Function_ref fav) {
    auto it = _function_memory_map.find(fav->getFunctionId());
    if (it == _function_memory_map.end()) {
      function_memory_location* ml =
          new function_memory_location(fav->getFunctionType(),
                                       fav->getFunctionId());
      _function_memory_map.emplace(fav->getFunctionId(),
                                 std::unique_ptr< function_memory_location >(
                                     ml));
      return ml;
    } else {
      return it->second.get();
    }
  }

  //! Get or create a global_memory_location
  aggregate_memory_location* get_aggregate(arbos::Internal_Variable_ref iv) {
    auto it = _aggregate_memory_map.find(arbos::ar::getName(iv));
    if (it ==_aggregate_memory_map.end()) {
      aggregate_memory_location* ml = new aggregate_memory_location(iv);
      _aggregate_memory_map
          .emplace(arbos::ar::getName(iv),
                   std::unique_ptr< aggregate_memory_location >(ml));
      return ml;
      } else {
        return it->second.get();
      }
  }

  //! Get or create a va_arg_memory_location
  va_arg_memory_location* get_va_arg(const std::string& sv) {
    auto it = _va_arg_memory_map.find(sv);
    if (it == _va_arg_memory_map.end()) {
      va_arg_memory_location* ml = new va_arg_memory_location(sv);
      _va_arg_memory_map.emplace(sv,
                                  std::unique_ptr< va_arg_memory_location >(ml));
      return ml;
      } else {
        return it->second.get();
      }
  }

  //! Get or create a fixed_address_memory_location
  fixed_address_memory_location* get_fixed_addr(ikos::z_number addr) {
    auto it = _fixed_address_memory_map.find(addr);
    if (it == _fixed_address_memory_map.end()) {
      fixed_address_memory_location* ml =
          new fixed_address_memory_location(addr);
      _fixed_address_memory_map
          .emplace(addr, std::unique_ptr< fixed_address_memory_location >(ml));
      return ml;
    } else {
      return it->second.get();
    }
  }

  //! Get or create a dyn_alloc_memory_location
  dyn_alloc_memory_location* get_dyn_alloc(arbos::Call_ref call_stmt) {
    auto it = _dyn_alloc_memory_map.find(call_stmt);
    if (it == _dyn_alloc_memory_map.end()) {
      dyn_alloc_memory_location* ml = new dyn_alloc_memory_location(call_stmt);
      _dyn_alloc_memory_map
          .emplace(call_stmt,
                   std::unique_ptr< dyn_alloc_memory_location >(ml));
      return ml;
    } else {
      return it->second.get();
    }
  }

}; // class memory_factory

} // end namespace analyzer

namespace ikos {

//! Index trait class specialization for memory_location*
/*!
 * The index of memory_location* is the address of the pointer, write trait
 * writes the index of the memory_location.
 */
template <>
struct index_traits< analyzer::memory_location* > {
  static index64_t index(const analyzer::memory_location* m) {
    static_assert(sizeof(index64_t) >= sizeof(std::intptr_t),
                  "index64_t must be at least the size of an intptr_t");
    return reinterpret_cast< index64_t >(m);
  }

  static void write(std::ostream& o, analyzer::memory_location* m) {
    o << m->str();
  }
};

} // end namespace ikos

namespace analyzer {

//! Specify the hash function of a memory_location*
inline std::size_t hash_value(const memory_location* ml) {
  return std::hash< ikos::index64_t >()(
      ikos::index_traits< memory_location* >::index(ml));
}

} // end namespace analyzer

namespace std {

template <>
struct hash< analyzer::memory_location* > {
  std::size_t operator()(const analyzer::memory_location* ml) const {
    return analyzer::hash_value(ml);
  }
};

} // end namespace std


#endif // ANALYZER_MEMORY_LOCATION_HPP
