/*******************************************************************************
 *
 * Declaration of classes used in ARBOS.
 *
 * Authors: Nija Shi
 *          Arnaud J. Venet
 *
 * Contributors: Maxime Arthaud
 *
 * Contact: ikos@lists.nasa.gov
 *
 * Notices:
 *
 * Copyright (c) 2011-2016 United States Government as represented by the
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

#ifndef ARBOS_AR_HPP
#define ARBOS_AR_HPP

#include <stdint.h>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/iterator/indirect_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/make_shared.hpp>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>

#include <arbos/common/common.hpp>
#include <arbos/io/s_expressions.hpp>

namespace arbos {

/**
 * parse_error prints s-expression parsing errors.
 */
class parse_error {
private:
  std::string _ar_type;
  s_expression _e;

private:
  parse_error();

public:
  parse_error(std::string ar_type, s_expression e) : _ar_type(ar_type), _e(e) {}

  std::string message() {
    std::ostringstream ss;
    ss << "[AR Parse Error] cannot parse " << _e << " to construct "
       << _ar_type;
    return ss.str();
  }

  void write(std::ostream& o) { o << this->message(); }

  virtual ~parse_error() {}

}; // class parse_error

inline std::ostream& operator<<(std::ostream& o, parse_error& e) {
  e.write(o);
  return o;
}

/**
 * UIDGenerator generates unique IDs to differentiate every single AR node used
 * in the AR model.
 */
class UIDGenerator {
private:
  static index64_t currentUID;

public:
  inline static index64_t nextUID() { return currentUID++; }

  inline static index64_t getCurrentUID() { return currentUID; }
};

const std::string EM_STR;

class AR_Node;
class Visitor;

// Enumeration of all AR class types.
enum AR_CLASS_TYPE_CODE {
  UNDEFINED_CLASS_TYPE,
  AR_BUNDLE_CLASS_TYPE,
  AR_FUNCTION_CLASS_TYPE,
  AR_CODE_CLASS_TYPE,
  AR_BASIC_BLOCK_CLASS_TYPE,
  AR_CONV_OP_STATEMENT_CLASS_TYPE,
  AR_BITWISE_OP_STATEMENT_CLASS_TYPE,
  AR_INTEGER_COMPARISON_STATEMENT_CLASS_TYPE,
  AR_FP_COMPARISON_STATEMENT_CLASS_TYPE,
  AR_ASSIGNMENT_STATEMENT_CLASS_TYPE,
  AR_ASSERTION_STATEMENT_CLASS_TYPE,
  AR_CALL_STATEMENT_CLASS_TYPE,
  AR_INVOKE_STATEMENT_CLASS_TYPE,
  AR_STORE_STATEMENT_CLASS_TYPE,
  AR_LOAD_STATEMENT_CLASS_TYPE,
  AR_LOAD_ELEMENT_CLASS_TYPE,
  AR_STORE_ELEMENT_CLASS_TYPE,
  AR_POINTER_SHIFT_STATEMENT_CLASS_TYPE,
  AR_RETURN_VALUE_STATEMENT_CLASS_TYPE,
  AR_INTEGER_ARITH_OP_STATEMENT_CLASS_TYPE,
  AR_FP_ARITH_OP_STATEMENT_CLASS_TYPE,
  AR_ABSTRACT_VAR_CLASS_TYPE,
  AR_ABSTRACT_MEM_CLASS_TYPE,
  AR_LANDING_PAD_CLASS_TYPE,
  AR_RESUME_CLASS_TYPE,
  AR_VA_START_STATEMENT_CLASS_TYPE,
  AR_VA_END_STATEMENT_CLASS_TYPE,
  AR_VA_COPY_STATEMENT_CLASS_TYPE,
  AR_VA_ARG_STATEMENT_CLASS_TYPE,
  AR_UNREACHABLE_STATEMENT_CLASS_TYPE,
  AR_NOP_STATEMENT_CLASS_TYPE,
  AR_MEMCPY_STATEMENT_CLASS_TYPE,
  AR_MEMMOVE_STATEMENT_CLASS_TYPE,
  AR_MEMSET_STATEMENT_CLASS_TYPE,
  AR_BOOLEAN_TYPE_CLASS_TYPE,
  AR_BOOLEAN_CONSTANT_CLASS_TYPE,
  AR_INTEGER_TYPE_CLASS_TYPE,
  AR_INTEGER_CONSTANT_CLASS_TYPE,
  AR_FLOAT_TYPE_CLASS_TYPE,
  AR_FLOAT_CONSTANT_CLASS_TYPE,
  AR_FUNCTION_TYPE_CLASS_TYPE,
  AR_POINTER_TYPE_CLASS_TYPE,
  AR_ARRAY_TYPE_CLASS_TYPE,
  AR_ARRAY_CONSTANT_CLASS_TYPE,
  AR_STRUCT_TYPE_CLASS_TYPE,
  AR_STRUCT_CONSTANT_CLASS_TYPE,
  AR_RANGE_CONSTANT_CLASS_TYPE,
  AR_AGGREGATEZERO_CONSTANT_CLASS_TYPE,
  AR_EXPRESSION_CONSTANT_CLASS_TYPE,
  AR_UNDEFINED_CONSTANT_CLASS_TYPE,
  AR_CST_OPERAND_CLASS_TYPE,
  AR_VAR_OPERAND_CLASS_TYPE,
  AR_FUNCTION_ADDR_CONSTANT_CLASS_TYPE,
  AR_VAR_ADDR_CONSTANT_CLASS_TYPE,
  AR_NULL_CONSTANT_CLASS_TYPE,
  AR_INTERNAL_VAR_CLASS_TYPE,
  AR_VOID_TYPE_CLASS_TYPE,
  AR_OPAQUE_TYPE_CLASS_TYPE,
  AR_LOCAL_VAR_CLASS_TYPE,
  AR_GLOBAL_VAR_CLASS_TYPE,
  AR_SOURCE_LOCATION_CLASS_TYPE
};

/**
 * AR_Node is the superclass of all AR nodes.
 */
class AR_Node : public std::enable_shared_from_this< AR_Node > {
private:
  index64_t _uid;

public:
  AR_Node();
  virtual ~AR_Node() {}

  //! Prints this AR_Node
  virtual void print(std::ostream&) = 0;

  //! Returns the UID of this AR_Node
  inline index64_t getUID() { return _uid; }

  //! Allows a Visitor to access this AR_Node
  virtual void accept(std::shared_ptr< Visitor > visitor) = 0;

  //! Returns the AR_CLASS_TYPE_CODE of this AR class type
  virtual AR_CLASS_TYPE_CODE getClassType() { return UNDEFINED_CLASS_TYPE; }

  std::shared_ptr< AR_Node > _this() { return shared_from_this(); }
};

class ARModelEventListener {
public:
  enum Event { NODESCREATED };
  virtual void nodesCreated() {}
  virtual ~ARModelEventListener() {}

  // This is only necessary for gcc <= 4.9.3
  // see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=60970
  struct EventHash {
    std::size_t operator()(const Event& e) const {
      return static_cast< std::size_t >(e);
    }
  };
};

class ReferenceCounter {
private:
  std::unordered_map< index64_t, int > _ref_count;
  static ReferenceCounter* _instance;

public:
  inline void print_ct(index64_t uid) {
    std::cerr << "ref_ct(#" << uid << ")=" << _ref_count[uid] << std::endl;
  }

  inline void increment_ct(index64_t uid) {
    if (_ref_count.find(uid) == _ref_count.end()) {
      _ref_count[uid] = 0;
    }

    _ref_count[uid]++;
  }

  inline void decrement_ct(index64_t uid) {
    if (_ref_count.find(uid) != _ref_count.end()) {
      _ref_count[uid]--;
      assert(_ref_count[uid] >= 0);
    } else {
      std::ostringstream ss;
      ss << "ReferenceCounter cannot find uid = " << uid;
      throw error(ss.str());
    }
  }

  inline int get_ct(index64_t uid) { return _ref_count[uid]; }

  static ReferenceCounter* Get() {
    if (_instance == NULL) {
      _instance = new ReferenceCounter();
    }

    return _instance;
  }

  std::string str(AR_CLASS_TYPE_CODE);

  ~ReferenceCounter() {}
};

/**
 * AR_Node_Ref implements smart pointer that references an AR_Node by its UID.
 */
template < typename T >
class AR_Node_Ref {
private:
  typedef AR_Node_Ref< T > AR_Node_Ref_t;
  index64_t _uid;

private:
  void swap(index64_t new_uid);

public:
  AR_Node_Ref< T >(const AR_Node_Ref_t& node_ref);
  AR_Node_Ref_t& operator=(const AR_Node_Ref_t& rhs);

  template < typename X >
  inline bool operator==(const AR_Node_Ref< X >& rhs) const {
    return _uid == rhs.getUID();
  }

  template < typename X >
  inline bool operator!=(const AR_Node_Ref< X >& rhs) const {
    return _uid != rhs.getUID();
  }

  template < typename X >
  inline bool operator<(const AR_Node_Ref< X >& rhs) const {
    return _uid < rhs.getUID();
  }

  T& operator*() const;
  T* operator->() const;
  explicit AR_Node_Ref(index64_t uid);
  AR_Node_Ref() : _uid(0) {}
  void setUID(index64_t uid);
  inline index64_t getUID() const { return _uid; }
  ~AR_Node_Ref< T >();
};

template < typename X, typename Y >
AR_Node_Ref< X > node_cast(const AR_Node_Ref< Y >& rhs) {
  return AR_Node_Ref< X >(rhs.getUID());
}

/*
 * A special iterator that goes through a container of index64_t and returns
 * AR_Node_Ref< X >
 */
template < typename X >
struct MkNodeRef : public std::unary_function< index64_t, AR_Node_Ref< X > > {
  AR_Node_Ref< X > operator()(index64_t uid) const {
    return AR_Node_Ref< X >(uid);
  }
};

template < typename X, typename Y >
struct node_iterator {
  typedef boost::transform_iterator< MkNodeRef< X >, typename Y::iterator >
      type;
};

/**
 * AR_Source_Location is an AR node that represents source location specified
 * with a line number in a file.
 */
class AR_Source_Location : public AR_Node {
private:
  index64_t _fid;
  std::string _filename;
  z_number _line;
  z_number _column;

private:
  AR_Source_Location(s_expression);

public:
  virtual ~AR_Source_Location();
  inline std::string getFilename() { return _filename; }
  inline z_number getLineNumber() { return _line; }
  inline z_number getColumnNumber() { return _column; }
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_SOURCE_LOCATION_CLASS_TYPE;
  }
  static std::shared_ptr< AR_Source_Location > create(s_expression e) {
    return std::static_pointer_cast< AR_Source_Location >(
        (new AR_Source_Location(e))->shared_from_this());
  }
};

////
// Types
////

enum Signedness { sign_unknown, sign_unsigned, sign_signed };

/**
 * AR_Type is the superclass of an AR node that represents a data type.
 */
class AR_Type : public AR_Node {
protected:
  z_number _real_size;
  z_number _store_size;

protected:
  AR_Type();

public:
  std::string getSignednessText(Signedness signedness);
  virtual ~AR_Type() {}
  virtual void print(std::ostream& out) = 0;
  virtual void accept(std::shared_ptr< Visitor > visitor) {}

  //! Returns the real size of the type in bits
  inline z_number getRealSize() { return _real_size; }

  //! Returns the stored size of the type in bytes
  inline z_number getStoreSize() { return _store_size; }
};

typedef node_iterator< AR_Type, std::vector< index64_t > >::type type_iterator;

/**
 * AR_Function_Type represents a function type as a method signature.
 */
class AR_Function_Type : public AR_Type {
private:
  index64_t _return_type;
  std::vector< index64_t > _param_types;
  bool _isVARARGS;

private:
  AR_Function_Type(s_expression);

public:
  inline AR_Node_Ref< AR_Type > getReturnType() {
    return AR_Node_Ref< AR_Type >(_return_type);
  }
  inline bool isVarargs() { return _isVARARGS; }
  virtual ~AR_Function_Type();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_FUNCTION_TYPE_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Function_Type
  static std::shared_ptr< AR_Function_Type > create(s_expression e) {
    return std::static_pointer_cast< AR_Function_Type >(
        (new AR_Function_Type(e))->shared_from_this());
  }
};

/**
 * AR_Integer_Type represents an integer type specified with signedness and size
 * in bits.
 */
class AR_Integer_Type : public AR_Type {
private:
  Signedness _signedness;

private:
  AR_Integer_Type(s_expression);

public:
  inline Signedness getSignedness() { return _signedness; }
  virtual ~AR_Integer_Type() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_INTEGER_TYPE_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Integer_Type
  static std::shared_ptr< AR_Integer_Type > create(s_expression e) {
    return std::static_pointer_cast< AR_Integer_Type >(
        (new AR_Integer_Type(e))->shared_from_this());
  }
};

enum FP_Size_Type {
  _half,
  _float,
  _double,
  _fp128,
  _x86_fp80,
  _ppc_fp128,
  _unknown
};

/**
 * AR_FP_Type represents a floating-point type specified with signedness and
 * size in bits.
 */
class AR_FP_Type : public AR_Type {
private:
  Signedness _signedness;
  std::string _fp_kind;

private:
  AR_FP_Type(s_expression);

public:
  inline Signedness getSignedness() { return _signedness; }
  virtual ~AR_FP_Type() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() { return AR_FLOAT_TYPE_CLASS_TYPE; }

  //! Returns a shared_ptr to AR_FP_Type
  static std::shared_ptr< AR_FP_Type > create(s_expression e) {
    return std::static_pointer_cast< AR_FP_Type >(
        (new AR_FP_Type(e))->shared_from_this());
  }
};

/**
 * AR_Void_Type represents void type.
 */
class AR_Void_Type : public AR_Type {
private:
  AR_Void_Type(s_expression);

public:
  virtual ~AR_Void_Type(){};
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() { return AR_VOID_TYPE_CLASS_TYPE; }

  //! Returns a shared_ptr to AR_Void_Type
  static std::shared_ptr< AR_Void_Type > create(s_expression e) {
    return std::static_pointer_cast< AR_Void_Type >(
        (new AR_Void_Type(e))->shared_from_this());
  }
};

/**
 * AR_Point_Type represents pointer type.
 */
class AR_Pointer_Type : public AR_Type {
private:
  index64_t _cell_type;

private:
  AR_Pointer_Type(s_expression);

public:
  inline AR_Node_Ref< AR_Type > getCellType() {
    return AR_Node_Ref< AR_Type >(_cell_type);
  }
  virtual ~AR_Pointer_Type();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_POINTER_TYPE_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Pointer_Type
  static std::shared_ptr< AR_Pointer_Type > create(s_expression e) {
    return std::static_pointer_cast< AR_Pointer_Type >(
        (new AR_Pointer_Type(e))->shared_from_this());
  }
};

/**
 * AR_Structure_Type represents struct type.
 */
class AR_Structure_Type : public AR_Type {
private:
  // The following properties of the struct are
  // determined by the LLVM ARBOS pass.
  static const std::string ALIGN_TAG;
  static const std::string SIZE_TAG;

  //
  std::vector< index64_t > _field_types;
  std::map< z_number, index64_t > _layout;
  z_number _alignment;

private:
  AR_Structure_Type(s_expression);

public:
  /**
   *  Returns offset to the field by its position.
   *  E.g., if field_position = 3, then offset
   *  is the 3rd pair in the map, layout.
   */
  z_number getOffsetOfField(int field_position);

  /**
   * Returns the size of the data type for a field
   * specified by its field position.
   */
  z_number getSizeOfField(int field_position);

  std::map< z_number, AR_Node_Ref< AR_Type > > getLayout() {
    std::map< z_number, AR_Node_Ref< AR_Type > > map;
    std::map< z_number, index64_t >::iterator p = _layout.begin();
    for (; p != _layout.end(); p++) {
      map[p->first] = AR_Node_Ref< AR_Type >(p->second);
    }
    return map;
  }

  std::pair< type_iterator, type_iterator > getAllFieldTypes();

  virtual ~AR_Structure_Type();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_STRUCT_TYPE_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Structure_Type
  static std::shared_ptr< AR_Structure_Type > create(s_expression e) {
    return std::static_pointer_cast< AR_Structure_Type >(
        (new AR_Structure_Type(e))->shared_from_this());
  }
  void print_layout(std::vector< index64_t >& visited, std::ostream& out);
};

/**
 * AR_Array_Type represnts array type.
 */
class AR_Array_Type : public AR_Type {
private:
  index64_t _cell_type;
  z_number _capacity;

private:
  AR_Array_Type(s_expression);

public:
  inline AR_Node_Ref< AR_Type > getCellType() {
    return AR_Node_Ref< AR_Type >(_cell_type);
  }
  inline z_number getCapacity() { return _capacity; }
  virtual ~AR_Array_Type();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() { return AR_ARRAY_TYPE_CLASS_TYPE; }

  //! Returns a shared_ptr to AR_Array_Type
  static std::shared_ptr< AR_Array_Type > create(s_expression e) {
    return std::static_pointer_cast< AR_Array_Type >(
        (new AR_Array_Type(e))->shared_from_this());
  }
};

/**
 * AR_Opaque_Type represents opaque type.
 */
class AR_Opaque_Type : public AR_Type {
private:
  AR_Opaque_Type(int size) : AR_Type() { _real_size = size; }

public:
  virtual ~AR_Opaque_Type() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_OPAQUE_TYPE_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Opaque_Type
  static std::shared_ptr< AR_Opaque_Type > create(int size) {
    return std::static_pointer_cast< AR_Opaque_Type >(
        (new AR_Opaque_Type(size))->shared_from_this());
  }
};

////
// Constants
////

/**
 * AR_Constant is the superclass that represent constant values.
 */
class AR_Constant : public AR_Node {
protected:
  AR_Constant() : AR_Node() {}

public:
  virtual ~AR_Constant(){};
  virtual void print(std::ostream& out) = 0;
  virtual void accept(std::shared_ptr< Visitor > visitor) = 0;
  virtual AR_Node_Ref< AR_Type > getType() = 0;
};

/**
 * AR_Undefined_Constant represents undefined that can be generated by compiler
 * frondend, such as LLVM.
 */
class AR_Undefined_Constant : public AR_Constant {
private:
  AR_Node_Ref< AR_Type > _type;

private:
  AR_Undefined_Constant(s_expression);
  AR_Undefined_Constant() : AR_Constant() {}

public:
  virtual ~AR_Undefined_Constant() {}
  virtual void print(std::ostream&);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_UNDEFINED_CONSTANT_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Undefined_Constant
  static std::shared_ptr< AR_Undefined_Constant > create(s_expression e) {
    return std::static_pointer_cast< AR_Undefined_Constant >(
        (new AR_Undefined_Constant(e))->shared_from_this());
  }

  //! Returns a shared_ptr to AR_Undefined_Constant
  static std::shared_ptr< AR_Undefined_Constant > create() {
    return std::static_pointer_cast< AR_Undefined_Constant >(
        (new AR_Undefined_Constant())->shared_from_this());
  }

  virtual AR_Node_Ref< AR_Type > getType() { return _type; }
};

/**
 * AR_Integer_Constant represents a constant of type AR_Integer_Type.
 */
class AR_Integer_Constant : public AR_Constant {
private:
  AR_Node_Ref< AR_Integer_Type > _type;
  z_number _value;

private:
  AR_Integer_Constant(s_expression);
  AR_Integer_Constant(z_number value, AR_Node_Ref< AR_Integer_Type > type)
      : AR_Constant(), _type(type), _value(value) {}

public:
  virtual ~AR_Integer_Constant();
  inline z_number getValue() { return _value; }
  virtual AR_Node_Ref< AR_Type > getType() {
    return node_cast< AR_Type >(_type);
  }
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_INTEGER_CONSTANT_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Integer_Constant
  static std::shared_ptr< AR_Integer_Constant > create(s_expression e) {
    return std::static_pointer_cast< AR_Integer_Constant >(
        (new AR_Integer_Constant(e))->shared_from_this());
  }
  static std::shared_ptr< AR_Integer_Constant > create(
      z_number v, AR_Node_Ref< AR_Integer_Type > ty) {
    return std::static_pointer_cast< AR_Integer_Constant >(
        (new AR_Integer_Constant(v, ty))->shared_from_this());
  }
};

/**
 * AR_FP_Constant represents a constant of type AR_FP_Type.
 */
class AR_FP_Constant : public AR_Constant {
private:
  AR_Node_Ref< AR_FP_Type > _type;
  fp_number _value;

private:
  AR_FP_Constant(s_expression);

public:
  inline fp_number getValue() { return _value; }
  virtual ~AR_FP_Constant();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_FLOAT_CONSTANT_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_FP_Constant
  static std::shared_ptr< AR_FP_Constant > create(s_expression e) {
    return std::static_pointer_cast< AR_FP_Constant >(
        (new AR_FP_Constant(e))->shared_from_this());
  }

  virtual AR_Node_Ref< AR_Type > getType() {
    return node_cast< AR_Type >(_type);
  }
};

/**
 * AR_Null_Constant represents NULL.
 */
class AR_Null_Constant : public AR_Constant {
private:
  AR_Node_Ref< AR_Type > _type;

private:
  AR_Null_Constant(s_expression);

public:
  virtual ~AR_Null_Constant();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_NULL_CONSTANT_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Null_Constant
  static std::shared_ptr< AR_Null_Constant > create(s_expression e) {
    return std::static_pointer_cast< AR_Null_Constant >(
        (new AR_Null_Constant(e))->shared_from_this());
  }

  virtual AR_Node_Ref< AR_Type > getType() { return _type; }
};

class AR_Operand;
class AR_Range_Constant : public AR_Constant {
private:
  AR_Node_Ref< AR_Type > _type;
  std::unordered_map< z_number, AR_Node_Ref< AR_Operand > > _values;

private:
  AR_Range_Constant(s_expression);

public:
  //! Returns the aggregate values
  inline const std::unordered_map< z_number, AR_Node_Ref< AR_Operand > >&
  getValues() const {
    return _values;
  }

  //! Returns the aggregate values
  inline std::unordered_map< z_number, AR_Node_Ref< AR_Operand > >&
  getValues() {
    return _values;
  }

  virtual ~AR_Range_Constant();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor) {}
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_RANGE_CONSTANT_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Range_Constant
  static std::shared_ptr< AR_Range_Constant > create(s_expression e) {
    return std::static_pointer_cast< AR_Range_Constant >(
        (new AR_Range_Constant(e))->shared_from_this());
  }
  virtual AR_Node_Ref< AR_Type > getType() { return _type; }
};

class AR_Function;

/**
 * AR_Function_Addr_Constant represents a function's address in memory.
 */
class AR_Function_Addr_Constant : public AR_Constant {
private:
  std::string _function_name;
  AR_Node_Ref< AR_Type > _type;

private:
  AR_Function_Addr_Constant(s_expression);

public:
  AR_Node_Ref< AR_Function > getFunction();
  inline std::string getFunctionName() { return _function_name; }
  virtual ~AR_Function_Addr_Constant();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_FUNCTION_ADDR_CONSTANT_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Function_Addr_Constant
  static std::shared_ptr< AR_Function_Addr_Constant > create(s_expression e) {
    return std::static_pointer_cast< AR_Function_Addr_Constant >(
        (new AR_Function_Addr_Constant(e))->shared_from_this());
  }

  virtual AR_Node_Ref< AR_Type > getType() { return _type; }
};

class AR_Variable;

/**
 * AR_Var_Addr_Constant represents a variable's address in memory.
 */
class AR_Var_Addr_Constant : public AR_Constant {
private:
  index64_t _variable;

private:
  AR_Var_Addr_Constant(s_expression);

public:
  inline AR_Node_Ref< AR_Variable > getVariable() {
    return AR_Node_Ref< AR_Variable >(_variable);
  }
  virtual ~AR_Var_Addr_Constant();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_VAR_ADDR_CONSTANT_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Var_Addr_Constant
  static std::shared_ptr< AR_Var_Addr_Constant > create(s_expression e) {
    return std::static_pointer_cast< AR_Var_Addr_Constant >(
        (new AR_Var_Addr_Constant(e))->shared_from_this());
  }

  virtual AR_Node_Ref< AR_Type > getType();
};

/////
// Variables
/////

/**
 * AR_Variable is the superclass of AR nodes that represent a global variable
 * and local variable.
 */
class AR_Variable : public AR_Node {
protected:
  std::string _name;
  AR_Node_Ref< AR_Source_Location > _source_location;
  AR_Node_Ref< AR_Type > _type;

protected:
  AR_Variable(const std::string& name, AR_Node_Ref< AR_Type > type)
      : AR_Node(), _name(name), _type(type) {}

  AR_Variable() : AR_Node() {}

public:
  virtual ~AR_Variable();
  virtual void print(std::ostream& out) = 0;
  virtual void accept(std::shared_ptr< Visitor > visitor) = 0;

  //! Returns the AR_Type of this AR_Variable
  inline AR_Node_Ref< AR_Type > getType() { return _type; }

  //! Returns the name of this AR_Variable
  inline const std::string& getNameId() { return _name; }

  //! Returns the AR_Source_Location where this AR_Variable was declared
  inline AR_Node_Ref< AR_Source_Location > getSourceLocation() {
    return _source_location;
  }

  // ! Prints source location
  void print_sourcelocation(std::ostream& out);
};

/**
 * AR_Local_Variable represents a local variable defined in an AR_Function.
 */
class AR_Local_Variable : public AR_Variable {
private:
  AR_Node_Ref< AR_Function > _function;

private:
  AR_Local_Variable(s_expression);
  AR_Local_Variable(const std::string& name, AR_Node_Ref< AR_Type > type)
      : AR_Variable(name, type) {}

public:
  //! Returns containing function
  inline AR_Node_Ref< AR_Function > getContainingFunction() {
    return _function;
  }

  virtual ~AR_Local_Variable();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() { return AR_LOCAL_VAR_CLASS_TYPE; }

  //! Returns a shared_ptr to AR_Local_Variable
  static std::shared_ptr< AR_Local_Variable > create(s_expression e) {
    return std::static_pointer_cast< AR_Local_Variable >(
        (new AR_Local_Variable(e))->shared_from_this());
  }

  //! Returns a shared_ptr to AR_Local_Variable
  static std::shared_ptr< AR_Local_Variable > create(
      const std::string& name, AR_Node_Ref< AR_Type > type) {
    return std::static_pointer_cast< AR_Local_Variable >(
        (new AR_Local_Variable(name, type))->shared_from_this());
  }
};

class AR_Code;

/**
 * AR_Global_Variable represents a global variable.
 */
class AR_Global_Variable : public AR_Variable {
private:
  AR_Node_Ref< AR_Code > _initializer;

private:
  AR_Global_Variable(s_expression);

public:
  /*! Returns the initializer code. This method can return an AR_Code with zero
   * UID.
   *  This means the global variable may have come with an implicit initializer
   * but
   *  is not carried over to the AR representation.
   */
  inline AR_Node_Ref< AR_Code > getInitializer() { return _initializer; }

  virtual ~AR_Global_Variable();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() { return AR_GLOBAL_VAR_CLASS_TYPE; }

  //! Returns a shared_ptr to AR_Global_Variable
  static std::shared_ptr< AR_Global_Variable > create(s_expression e) {
    return std::static_pointer_cast< AR_Global_Variable >(
        (new AR_Global_Variable(e))->shared_from_this());
  }
};

/**
 * AR_Internal_Variable represents an internal variable. An internal variable
 * can be synthesized by a compiler frontend, such as LLVM.
 */
class AR_Internal_Variable : public AR_Node {
private:
  std::string _name;
  AR_Node_Ref< AR_Type > _type;

private:
  AR_Internal_Variable(s_expression);
  AR_Internal_Variable(const std::string& name, AR_Node_Ref< AR_Type > type)
      : AR_Node(), _name(name), _type(type) {}

public:
  //! Returns the type of the internal variable
  inline AR_Node_Ref< AR_Type > getType() { return _type; }

  //! Returns the name of the internal variable
  inline const std::string& getName() { return _name; }

  virtual ~AR_Internal_Variable();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_INTERNAL_VAR_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Internal_Variable
  static std::shared_ptr< AR_Internal_Variable > create(s_expression e) {
    return std::static_pointer_cast< AR_Internal_Variable >(
        (new AR_Internal_Variable(e))->shared_from_this());
  }
  static std::shared_ptr< AR_Internal_Variable > create(
      const std::string& name, AR_Node_Ref< AR_Type > type) {
    return std::static_pointer_cast< AR_Internal_Variable >(
        (new AR_Internal_Variable(name, type))->shared_from_this());
  }
};

/////
// Operands
/////

/**
 * AR_Operand is a operand that serves as a wrapper of an internal variable or a
 * constant. An AR_Operand is used as an operand in an AR_Statement.
 */
class AR_Operand : public AR_Node {
protected:
  AR_Operand() : AR_Node() {}

public:
  virtual ~AR_Operand() {}
  virtual void print(std::ostream& out) = 0;
  virtual void accept(std::shared_ptr< Visitor > visitor) = 0;
  virtual AR_Node_Ref< AR_Type > getType() = 0;
};

/**
 * AR_Var_Operand wraps an AR_Internal_Variable.
 */
class AR_Var_Operand : public AR_Operand {
private:
  AR_Node_Ref< AR_Internal_Variable > _internal_variable;

private:
  AR_Var_Operand(s_expression);
  AR_Var_Operand(AR_Node_Ref< AR_Internal_Variable > v)
      : AR_Operand(), _internal_variable(v) {}

public:
  //! Return the internal variable
  inline AR_Node_Ref< AR_Internal_Variable > getInternalVariable() {
    return _internal_variable;
  }

  virtual ~AR_Var_Operand();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_VAR_OPERAND_CLASS_TYPE;
  }

  //! Return the shared_ptr to AR_Var_Operand
  static std::shared_ptr< AR_Var_Operand > create(s_expression e) {
    return std::static_pointer_cast< AR_Var_Operand >(
        (new AR_Var_Operand(e))->shared_from_this());
  }
  static std::shared_ptr< AR_Var_Operand > create(
      AR_Node_Ref< AR_Internal_Variable > v) {
    return std::static_pointer_cast< AR_Var_Operand >(
        (new AR_Var_Operand(v))->shared_from_this());
  }

  virtual AR_Node_Ref< AR_Type > getType() {
    return (*_internal_variable).getType();
  }
};

/**
 * AR_Cst_Operand wraps an AR_Constant.
 */
class AR_Cst_Operand : public AR_Operand {
private:
  AR_Node_Ref< AR_Constant > _constant;

private:
  AR_Cst_Operand(s_expression);
  AR_Cst_Operand(AR_Node_Ref< AR_Constant > constant)
      : AR_Operand(), _constant(constant) {}

public:
  //! Returns the constant
  inline AR_Node_Ref< AR_Constant > getConstant() { return _constant; }

  virtual ~AR_Cst_Operand();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_CST_OPERAND_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Cst_Operand
  static std::shared_ptr< AR_Cst_Operand > create(s_expression e) {
    return std::static_pointer_cast< AR_Cst_Operand >(
        (new AR_Cst_Operand(e))->shared_from_this());
  }
  static std::shared_ptr< AR_Cst_Operand > create(
      AR_Node_Ref< AR_Constant > c) {
    return std::static_pointer_cast< AR_Cst_Operand >(
        (new AR_Cst_Operand(c))->shared_from_this());
  }
  virtual AR_Node_Ref< AR_Type > getType() { return (*_constant).getType(); }
};

////
// Statements
////

class AR_Basic_Block;
typedef node_iterator< AR_Basic_Block, std::vector< index64_t > >::type
    bblock_iterator;

/**
 * AR_Statement is the superclass of an AR node that represents a statement.
 */
class AR_Statement : public AR_Node {
protected:
  //! UID of the containing AR_Basic_Block
  index64_t _parent_bblock_uid;

  //! Source location where this AR_Statement corresponds to
  AR_Node_Ref< AR_Source_Location > _source_location;

protected:
  AR_Statement(index64_t parent_bblock_uid)
      : AR_Node(), _parent_bblock_uid(parent_bblock_uid) {}
  void print_sourcelocation(std::ostream& out);

public:
  //! A destructor
  virtual ~AR_Statement();

  //! Prints this AR_Statement
  virtual void print(std::ostream& out) = 0;

  //! Allows a Visitor to access this AR_Statement
  virtual void accept(std::shared_ptr< Visitor > visitor) = 0;

  //! Returns the AR_Basic_Block that contains this AR_Statement.
  AR_Node_Ref< AR_Basic_Block > getContainingBasicBlock();

  //! Sets the AR_Basic_Block that contains this AR_Statement.
  inline void setContainingBasicBlock(index64_t new_parent_bblock_uid) {
    _parent_bblock_uid = new_parent_bblock_uid;
  }

  //! Returns the AR_Source_Location of this AR_Statement.
  inline AR_Node_Ref< AR_Source_Location > getSourceLocation() {
    return _source_location;
  }

  //! Sets the AR_Source_Location of this AR_Statement.
  void setSourceLocation(AR_Node_Ref< AR_Source_Location > src_loc) {
    _source_location = src_loc;
  }
};

enum ArithOp { add, sub, mul, udiv, sdiv, urem, srem };

/**
 * AR_Arith_Op is an AR statement that represents the following arithmic
 * operations: add, sub, mul, udiv, sdiv, urem, srem.
 */
class AR_Arith_Op : public AR_Statement, public ARModelEventListener {
private:
  ArithOp _op;
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _left_op, _right_op;

private:
  AR_Arith_Op(index64_t parent_bblock, s_expression, ArithOp opType);

public:
  virtual ~AR_Arith_Op();

  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the arithmetic operation in string
  std::string getArithOpText();

  //! Returns the arithmetic operation enum
  inline ArithOp getArithOp() { return _op; }

  //! Returns the result
  inline AR_Node_Ref< AR_Internal_Variable > getResult() { return _result; }

  //! Returns the left operand
  inline AR_Node_Ref< AR_Operand > getLeftOperand() { return _left_op; }

  //! Returns the right operand
  inline AR_Node_Ref< AR_Operand > getRightOperand() { return _right_op; }

  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_INTEGER_ARITH_OP_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Arith_Op
  static std::shared_ptr< AR_Arith_Op > create(index64_t parent_bblock,
                                               s_expression e,
                                               ArithOp opType) {
    return std::static_pointer_cast< AR_Arith_Op >(
        (new AR_Arith_Op(parent_bblock, e, opType))->shared_from_this());
  }

  //! Sets result
  inline void setResult(AR_Node_Ref< AR_Internal_Variable > o) { _result = o; }

  //! Sets the left operand
  inline void setLeftOperand(AR_Node_Ref< AR_Operand > o) { _left_op = o; }

  //! Sets the right operand
  inline void setRightOperand(AR_Node_Ref< AR_Operand > o) { _right_op = o; }
};

enum CompOp { eq, ne, ult, ule, slt, sle, i_unknown };

/**
 * AR_Integer_Comparison represents an assertion statement for integer type
 * comparison.
 */
class AR_Integer_Comparison : public AR_Statement {
private:
  AR_Node_Ref< AR_Operand > _left_op, _right_op;
  bool _continue_if_true;
  CompOp _predicate;

private:
  AR_Integer_Comparison(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_Integer_Comparison() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns whether this comparison operation continues if true
  inline bool isContinueIfTrue() { return _continue_if_true; }

  //! Returns the comparison operation in string
  std::string getCompOpText();

  //! Returns the left operand
  inline AR_Node_Ref< AR_Operand > getLeftOperand() { return _left_op; }

  //! Returns the right operand
  inline AR_Node_Ref< AR_Operand > getRightOperand() { return _right_op; }

  //! Returns the predicate enum
  inline CompOp getPredicate() { return _predicate; }

  static CompOp translate(const std::string& pred_text);

  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_INTEGER_COMPARISON_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Integer_Comparison
  static std::shared_ptr< AR_Integer_Comparison > create(
      index64_t parent_bblock, s_expression e) {
    return std::static_pointer_cast< AR_Integer_Comparison >(
        (new AR_Integer_Comparison(parent_bblock, e))->shared_from_this());
  }

  //! Sets the left operand
  inline void setLeftOperand(AR_Node_Ref< AR_Operand > o) { _left_op = o; }

  //! Sets the right operand
  inline void setRightOperand(AR_Node_Ref< AR_Operand > o) { _right_op = o; }
};

enum FPOp { fadd, fsub, fmul, fdiv, frem };

/**
 * AR_FP_Op represents the following floating-point type arithmic operations:
 * fadd, fsub, fmul, fdiv, frem.
 */
class AR_FP_Op : public AR_Statement {
private:
  FPOp _op;
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _left_op;
  AR_Node_Ref< AR_Operand > _right_op;

private:
  AR_FP_Op(index64_t parent_bblock, s_expression, FPOp opType);

public:
  virtual ~AR_FP_Op();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  std::string getFPOpText();
  inline FPOp getFPOp() { return _op; }

  //! Returns the result
  inline AR_Node_Ref< AR_Internal_Variable > getResult() { return _result; }

  //! Returns the left operand
  inline AR_Node_Ref< AR_Operand > getLeftOperand() { return _left_op; }

  //! Returns the right operand
  inline AR_Node_Ref< AR_Operand > getRightOperand() { return _right_op; }
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_FP_ARITH_OP_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_FP_Op
  static std::shared_ptr< AR_FP_Op > create(index64_t parent_bblock,
                                            s_expression e,
                                            FPOp opType) {
    return std::static_pointer_cast< AR_FP_Op >(
        (new AR_FP_Op(parent_bblock, e, opType))->shared_from_this());
  }

  //! Sets the result
  inline void setResult(AR_Node_Ref< AR_Internal_Variable > o) { _result = o; }

  //! Sets the left operand
  inline void setLeftOperand(AR_Node_Ref< AR_Operand > o) { _left_op = o; }

  //! Sets the right operand
  inline void setRightOperand(AR_Node_Ref< AR_Operand > o) { _right_op = o; }
};

enum FP_CompOp {
  oeq,
  olt,
  ole,
  one,
  ord,
  ueq,
  _ult,
  _ule,
  une,
  uno,
  fp_unknown
};

/**
 * AR_FP_Comparison represents an assertion statement for floating-point type
 * comparison.
 */
class AR_FP_Comparison : public AR_Statement {
private:
  AR_Node_Ref< AR_Operand > _left_op, _right_op;
  bool _continue_if_true;
  FP_CompOp _predicate;

private:
  AR_FP_Comparison(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_FP_Comparison() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the condition
  inline bool isContinueIfTrue() { return _continue_if_true; }

  //! Returns the textual format of the operation
  std::string getCompOpText();

  //! Returns the left operand
  inline AR_Node_Ref< AR_Operand > getLeftOperand() { return _left_op; }

  //! Returns the right operand
  inline AR_Node_Ref< AR_Operand > getRightOperand() { return _right_op; }

  //! Returns the predicate: oeq, olt, ole, one, ord, ueq, _ult, _ule, une, uno,
  // fp_unknown
  inline FP_CompOp getPredicate() { return _predicate; }

  //! Returns the corresponding enum of the operation
  static FP_CompOp translate(const std::string& pred_text);

  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_FP_COMPARISON_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_FP_Comparison
  static std::shared_ptr< AR_FP_Comparison > create(index64_t parent_bblock,
                                                    s_expression e) {
    return std::static_pointer_cast< AR_FP_Comparison >(
        (new AR_FP_Comparison(parent_bblock, e))->shared_from_this());
  }

  //! Sets the left operand
  inline void setLeftOperand(AR_Node_Ref< AR_Operand > o) { _left_op = o; }

  //! Sets the right operand
  inline void setRightOperand(AR_Node_Ref< AR_Operand > o) { _right_op = o; }
};

/**
 * AR_Assignment represents an assignment statement where the left-hand-side is
 * an AR_Internal_Variable.
 */
class AR_Assignment : public AR_Statement {
private:
  // AR_Type *type; => can be accessed via left operand
  AR_Node_Ref< AR_Internal_Variable > _left_op;
  AR_Node_Ref< AR_Operand > _right_op;

private:
  AR_Assignment(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_Assignment();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the type of the lhs
  inline AR_Node_Ref< AR_Type > getType() { return (*_left_op).getType(); }

  //! Returns the lhs
  inline AR_Node_Ref< AR_Internal_Variable > getLeftOperand() {
    return _left_op;
  }

  //! Returns the rhs
  inline AR_Node_Ref< AR_Operand > getRightOperand() { return _right_op; }
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_ASSIGNMENT_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Assignment
  static std::shared_ptr< AR_Assignment > create(index64_t parent_bblock,
                                                 s_expression e) {
    return std::static_pointer_cast< AR_Assignment >(
        (new AR_Assignment(parent_bblock, e))->shared_from_this());
  }

  //! Sets the left operand
  inline void setLeftOperand(AR_Node_Ref< AR_Internal_Variable > o) {
    _left_op = o;
  }

  //! Sets the right operand
  inline void setRightOperand(AR_Node_Ref< AR_Operand > o) { _right_op = o; }
};

enum ConvOp {
  trunc,
  zext,
  sext,
  fptrunc,
  fpext,
  fptoui,
  fptosi,
  uitofp,
  sitofp,
  ptrtoint,
  inttoptr,
  bitcast
};

/**
 * AR_Conv_Op represents the following type-casting operations: trunc, zext,
 sext, fptrunc, fpext, fptoui, fptosi, uitofp, sitofp,
 ptrtoint, inttoptr, bitcast.
 */
class AR_Conv_Op : public AR_Statement {
private:
  ConvOp _op;
  AR_Node_Ref< AR_Operand > _operand;
  AR_Node_Ref< AR_Operand > _result;

private:
  AR_Conv_Op(index64_t parent_bblock, s_expression, ConvOp opType);

public:
  //! Returns the rhs
  inline AR_Node_Ref< AR_Operand > getOperand() { return _operand; }

  //! Returns the result
  inline AR_Node_Ref< AR_Operand > getResult() { return _result; }

  //! Returns the textual representation of the operation
  std::string getConvOpText(ConvOp);

  //! Returns the enum of the operation
  inline ConvOp getConvOp() { return _op; }

  virtual ~AR_Conv_Op();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_CONV_OP_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Conv_Op
  static std::shared_ptr< AR_Conv_Op > create(index64_t parent_bblock,
                                              s_expression e,
                                              ConvOp opType) {
    return std::static_pointer_cast< AR_Conv_Op >(
        (new AR_Conv_Op(parent_bblock, e, opType))->shared_from_this());
  }

  //! Sete the right-hand-side operand
  inline void setOperand(AR_Node_Ref< AR_Operand > o) { _operand = o; }

  //! Sets the result
  inline void setResult(AR_Node_Ref< AR_Operand > o) { _result = o; }
};

enum BitwiseOp { _shl, _lshr, _ashr, _and, _or, _xor };

/**
 * AR_Bitwise_Op represents the following bitwise operations: _shl, _lshr,
 * _ashr, _and, _or, _xor.
 */
class AR_Bitwise_Op : public AR_Statement {
private:
  BitwiseOp _op;
  AR_Node_Ref< AR_Type > _type;
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _left_op;
  AR_Node_Ref< AR_Operand > _right_op;

private:
  AR_Bitwise_Op(index64_t parent_bblock, s_expression, BitwiseOp opType);

public:
  virtual ~AR_Bitwise_Op();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the name of the operation
  std::string getBitwiseOpText();

  //! Returns the enum of the operation
  inline BitwiseOp getBitwiseOp() { return _op; }

  //! Returns the result stored in AR_Internal_Variable
  inline AR_Node_Ref< AR_Internal_Variable > getResult() { return _result; }

  //! Returns the left operand
  inline AR_Node_Ref< AR_Operand > getLeftOperand() { return _left_op; }

  //! Returns the right operand
  inline AR_Node_Ref< AR_Operand > getRightOperand() { return _right_op; }
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_BITWISE_OP_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Bitwise_Op
  static std::shared_ptr< AR_Bitwise_Op > create(index64_t parent_bblock,
                                                 s_expression e,
                                                 BitwiseOp opType) {
    return std::static_pointer_cast< AR_Bitwise_Op >(
        (new AR_Bitwise_Op(parent_bblock, e, opType))->shared_from_this());
  }

  //! Sets the result
  inline void setResult(AR_Node_Ref< AR_Internal_Variable > o) { _result = o; }

  //! Sets the left operand
  inline void setLeftOperand(AR_Node_Ref< AR_Operand > o) { _left_op = o; }

  //! Sets the right operand
  inline void setRightOperand(AR_Node_Ref< AR_Operand > o) { _right_op = o; }
};

/**
 * AR_Store represents the opreation that stores a value from a register to a
 * memory address.
 */
class AR_Store : public AR_Statement {
private:
  AR_Node_Ref< AR_Operand > _value;
  AR_Node_Ref< AR_Operand > _ptr;

private:
  AR_Store(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_Store();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the pointer
  inline AR_Node_Ref< AR_Operand > getPointer() { return _ptr; }

  //! Returns the value to be stored
  inline AR_Node_Ref< AR_Operand > getValue() { return _value; }

  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_STORE_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Store
  static std::shared_ptr< AR_Store > create(index64_t parent_bblock,
                                            s_expression e) {
    return std::static_pointer_cast< AR_Store >(
        (new AR_Store(parent_bblock, e))->shared_from_this());
  }

  //! Sets the pointer
  inline void setPointer(AR_Node_Ref< AR_Operand > o) { _ptr = o; }

  //! Sets the value
  inline void setValue(AR_Node_Ref< AR_Operand > o) { _value = o; }
};

/**
 * AR_Load represents the operation that loads a the content located in memory
 * to a register.
 */
class AR_Load : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _ptr;

private:
  AR_Load(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_Load();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the lhs
  inline AR_Node_Ref< AR_Internal_Variable > getResult() { return _result; }

  //! Returns the pointer
  inline AR_Node_Ref< AR_Operand > getPointer() { return _ptr; }

  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_LOAD_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Load
  static std::shared_ptr< AR_Load > create(index64_t parent_bblock,
                                           s_expression e) {
    return std::static_pointer_cast< AR_Load >(
        (new AR_Load(parent_bblock, e))->shared_from_this());
  }

  //! Sets the result
  inline void setResult(AR_Node_Ref< AR_Internal_Variable > o) { _result = o; }

  //! Sets the pointer
  inline void setPointer(AR_Node_Ref< AR_Operand > o) { _ptr = o; }
};

/**
 * AR_Load_Element represents the operation that loads an element to an aggreate
 * in a register.
 */
class AR_Load_Element : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _value;
  AR_Node_Ref< AR_Operand > _offset;

private:
  AR_Load_Element(index64_t parent_bblock, s_expression);

public:
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_LOAD_ELEMENT_CLASS_TYPE;
  }
  virtual ~AR_Load_Element() {}

  //! Returns the result
  inline AR_Node_Ref< AR_Internal_Variable > getResult() { return _result; }

  //! Returns the value
  inline AR_Node_Ref< AR_Operand > getValue() { return _value; }

  //! Returns the offset
  inline AR_Node_Ref< AR_Operand > getOffset() { return _offset; }

  //! Returns the shared_ptr to AR_Load_Element
  static std::shared_ptr< AR_Load_Element > create(index64_t parent_bblock,
                                                   s_expression e) {
    return std::static_pointer_cast< AR_Load_Element >(
        (new AR_Load_Element(parent_bblock, e))->shared_from_this());
  }
};

/**
 * AR_Store_Element represents the operation that stores a single element from
 * an aggreate in a register.
 */
class AR_Store_Element : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _value;
  AR_Node_Ref< AR_Operand > _offset;
  AR_Node_Ref< AR_Type > _element_type;
  AR_Node_Ref< AR_Operand > _element;

private:
  AR_Store_Element(index64_t parent_bblock, s_expression);

public:
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual ~AR_Store_Element() {}
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_STORE_ELEMENT_CLASS_TYPE;
  }
  inline AR_Node_Ref< AR_Internal_Variable > getResult() { return _result; }
  inline AR_Node_Ref< AR_Operand > getValue() { return _value; }
  inline AR_Node_Ref< AR_Operand > getElement() { return _element; }
  inline AR_Node_Ref< AR_Operand > getOffset() { return _offset; }

  //! Returns the shared_ptr to AR_Store_Element
  static std::shared_ptr< AR_Store_Element > create(index64_t parent_bblock,
                                                    s_expression e) {
    return std::static_pointer_cast< AR_Store_Element >(
        (new AR_Store_Element(parent_bblock, e))->shared_from_this());
  }
};

/**
 * AR_NOP represents no-op.
 */
class AR_NOP : public AR_Statement {
private:
  AR_NOP(index64_t parent_bblock) : AR_Statement(parent_bblock) {}

public:
  virtual ~AR_NOP();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_NOP_STATEMENT_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Unreachable
  static std::shared_ptr< AR_NOP > create(index64_t parent_bblock,
                                          s_expression e) {
    return std::static_pointer_cast< AR_NOP >(
        (new AR_NOP(parent_bblock))->shared_from_this());
  }
};

/**
 * AR_Unreachable represents an unreachable statement generated by a compiler
 * frontend, such as LLVM.
 */
class AR_Unreachable : public AR_Statement {
private:
  AR_Unreachable(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_Unreachable();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_UNREACHABLE_STATEMENT_CLASS_TYPE;
  }

  //! Returns a shared_ptr to AR_Unreachable
  static std::shared_ptr< AR_Unreachable > create(index64_t parent_bblock,
                                                  s_expression e) {
    return std::static_pointer_cast< AR_Unreachable >(
        (new AR_Unreachable(parent_bblock, e))->shared_from_this());
  }
};

/**
 * AR_Return_Value represents the return-value statement generated by a compiler
 * frontend, such as LLVM.
 */
class AR_Return_Value : public AR_Statement {
private:
  AR_Node_Ref< AR_Operand > _value;

private:
  AR_Return_Value(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_Return_Value();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the value to be returned
  inline AR_Node_Ref< AR_Operand > getValue() { return _value; }

  //! Returns the type of the returned value
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_RETURN_VALUE_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Return_Value
  static std::shared_ptr< AR_Return_Value > create(index64_t parent_bblock,
                                                   s_expression e) {
    return std::static_pointer_cast< AR_Return_Value >(
        (new AR_Return_Value(parent_bblock, e))->shared_from_this());
  }

  //! Sets the returned value
  inline void setValue(AR_Node_Ref< AR_Operand > o) { _value = o; }
};

/**
 * AR_MemCpy represents the memcpy operation.
 */
class AR_MemCpy : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _tgt_pointer;
  AR_Node_Ref< AR_Operand > _src_pointer;
  AR_Node_Ref< AR_Operand > _len;

private:
  AR_MemCpy(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_MemCpy();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the destination pointer
  inline AR_Node_Ref< AR_Operand > getTarget() { return _tgt_pointer; }

  //! Returns the source pointer
  inline AR_Node_Ref< AR_Operand > getSource() { return _src_pointer; }

  //! Returns the length of the content to be copied
  inline AR_Node_Ref< AR_Operand > getLength() { return _len; }
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_MEMCPY_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_MemCpy
  static std::shared_ptr< AR_MemCpy > create(index64_t parent_bblock,
                                             s_expression e) {
    return std::static_pointer_cast< AR_MemCpy >(
        (new AR_MemCpy(parent_bblock, e))->shared_from_this());
  }

  //! Sets the target
  inline void setTarget(AR_Node_Ref< AR_Operand > o) { _tgt_pointer = o; }

  //! Sets the source
  inline void setSource(AR_Node_Ref< AR_Operand > o) { _src_pointer = o; }

  //! Sets the length
  inline void setLength(AR_Node_Ref< AR_Operand > o) { _len = o; }
};

/**
 * AR_MemMove represents the memcpy operation.
 */
class AR_MemMove : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _tgt_pointer;
  AR_Node_Ref< AR_Operand > _src_pointer;
  AR_Node_Ref< AR_Operand > _len;

private:
  AR_MemMove(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_MemMove();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the destination pointer
  inline AR_Node_Ref< AR_Operand > getTarget() { return _tgt_pointer; }

  //! Returns the source pointer
  inline AR_Node_Ref< AR_Operand > getSource() { return _src_pointer; }

  //! Returns the length of the content to be moved in memory
  inline AR_Node_Ref< AR_Operand > getLength() { return _len; }

  //! Returns the shared_ptr to AR_MemMove
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_MEMMOVE_STATEMENT_CLASS_TYPE;
  }
  static std::shared_ptr< AR_MemMove > create(index64_t parent_bblock,
                                              s_expression e) {
    return std::static_pointer_cast< AR_MemMove >(
        (new AR_MemMove(parent_bblock, e))->shared_from_this());
  }

  //! Sets the target
  inline void setTarget(AR_Node_Ref< AR_Operand > o) { _tgt_pointer = o; }

  //! Sets the source
  inline void setSource(AR_Node_Ref< AR_Operand > o) { _src_pointer = o; }

  //! Sets the length
  inline void setLength(AR_Node_Ref< AR_Operand > o) { _len = o; }
};

/**
 * AR_MemSet represents the memcpy operation.
 */
class AR_MemSet : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _ptr;
  AR_Node_Ref< AR_Operand > _value;
  AR_Node_Ref< AR_Operand > _len;

private:
  AR_MemSet(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_MemSet();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the length of the content to be initialized
  inline AR_Node_Ref< AR_Operand > getLength() { return _len; }

  //! Returns the pointer in memory
  inline AR_Node_Ref< AR_Operand > getOperand() { return _ptr; }

  //! Returns the value to be set
  inline AR_Node_Ref< AR_Operand > getValue() { return _value; }

  //! Returns the shared_ptr to AR_MemSet
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_MEMSET_STATEMENT_CLASS_TYPE;
  }
  static std::shared_ptr< AR_MemSet > create(index64_t parent_bblock,
                                             s_expression e) {
    return std::static_pointer_cast< AR_MemSet >(
        (new AR_MemSet(parent_bblock, e))->shared_from_this());
  }

  //! Sets the length
  inline void setLength(AR_Node_Ref< AR_Operand > o) { _len = o; }

  //! Sets the pointer as an operand
  inline void setOperand(AR_Node_Ref< AR_Operand > o) { _ptr = o; }

  //! Sets the value
  inline void setValue(AR_Node_Ref< AR_Operand > o) { _value = o; }
};

class AR_VA_Start : public AR_Statement {
private:
  AR_Node_Ref< AR_Operand > _va_list_ptr;

private:
  AR_VA_Start(index64_t parent_bblock, s_expression);

public:
  //! Returns ptr to the va_list
  inline AR_Node_Ref< AR_Operand > getPointer() { return _va_list_ptr; }

  virtual ~AR_VA_Start() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_VA_START_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_VA_Start
  static std::shared_ptr< AR_VA_Start > create(index64_t parent_bblock,
                                               s_expression e) {
    return std::static_pointer_cast< AR_VA_Start >(
        (new AR_VA_Start(parent_bblock, e))->shared_from_this());
  }
};

class AR_VA_End : public AR_Statement {
private:
  AR_Node_Ref< AR_Operand > _va_list_ptr;

private:
  AR_VA_End(index64_t parent_bblock, s_expression);

public:
  //! Returns ptr to the va_list
  inline AR_Node_Ref< AR_Operand > getPointer() { return _va_list_ptr; }

  virtual ~AR_VA_End() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_VA_END_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_VA_End
  static std::shared_ptr< AR_VA_End > create(index64_t parent_bblock,
                                             s_expression e) {
    return std::static_pointer_cast< AR_VA_End >(
        (new AR_VA_End(parent_bblock, e))->shared_from_this());
  }
};

class AR_VA_Copy : public AR_Statement {
private:
  AR_Node_Ref< AR_Operand > _dest_va_list_ptr;
  AR_Node_Ref< AR_Operand > _src_va_list_ptr;

private:
  AR_VA_Copy(index64_t parent_bblock, s_expression);

public:
  //! Returns ptr to the va_list
  inline AR_Node_Ref< AR_Operand > getDestination() {
    return _dest_va_list_ptr;
  }

  //! Returns ptr to the va_list
  inline AR_Node_Ref< AR_Operand > getSource() { return _src_va_list_ptr; }

  virtual ~AR_VA_Copy() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_VA_COPY_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_VA_Copy
  static std::shared_ptr< AR_VA_Copy > create(index64_t parent_bblock,
                                              s_expression e) {
    return std::static_pointer_cast< AR_VA_Copy >(
        (new AR_VA_Copy(parent_bblock, e))->shared_from_this());
  }
};

class AR_VA_Arg : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _result;
  AR_Node_Ref< AR_Operand > _va_list_ptr;
  AR_Node_Ref< AR_Type > _type;

private:
  AR_VA_Arg(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_VA_Arg() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_VA_ARG_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_VA_Arg
  static std::shared_ptr< AR_VA_Arg > create(index64_t parent_bblock,
                                             s_expression e) {
    return std::static_pointer_cast< AR_VA_Arg >(
        (new AR_VA_Arg(parent_bblock, e))->shared_from_this());
  }
};

/**
 * AR_Pointer_Shift represents a pointer-shifting operation.
 */
class AR_Pointer_Shift : public AR_Statement {
private:
  AR_Node_Ref< AR_Operand > _ptr;
  AR_Node_Ref< AR_Operand > _offset;
  AR_Node_Ref< AR_Internal_Variable > _result;

private:
  AR_Pointer_Shift(index64_t parent_bblock, s_expression);

public:
  virtual ~AR_Pointer_Shift();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);

  //! Returns the pointer
  inline AR_Node_Ref< AR_Operand > getPointer() { return _ptr; }

  //! Returns the offset
  inline AR_Node_Ref< AR_Operand > getOffset() { return _offset; }

  //! Returns the result of the address in an AR_Internal_Variable
  inline AR_Node_Ref< AR_Internal_Variable > getResult() { return _result; }
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_POINTER_SHIFT_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Pointer_Shift
  static std::shared_ptr< AR_Pointer_Shift > create(index64_t parent_bblock,
                                                    s_expression e) {
    return std::static_pointer_cast< AR_Pointer_Shift >(
        (new AR_Pointer_Shift(parent_bblock, e))->shared_from_this());
  }

  //! Sets the pointer
  inline void setPointer(AR_Node_Ref< AR_Operand > o) { _ptr = o; }

  //! Sets the offset
  inline void setOffset(AR_Node_Ref< AR_Operand > o) { _offset = o; }

  //! Sets the result
  inline void setResult(AR_Node_Ref< AR_Internal_Variable > o) { _result = o; }
};

/**
 * AR_Call represents a function call.
 */
class AR_Call : public AR_Statement {
private:
  std::vector< AR_Node_Ref< AR_Operand > > _args;
  // Note: NULL iff no return value used
  AR_Node_Ref< AR_Internal_Variable > _return_value;
  // Called function
  AR_Node_Ref< AR_Operand > _called_function;
  bool _is_inline_asm;

private:
  AR_Call(index64_t parent_bblock, s_expression);

public:
  //! Returns the called function
  AR_Node_Ref< AR_Operand > getFunction() { return _called_function; }

  //! Returns the arguments passed in this call
  inline const std::vector< AR_Node_Ref< AR_Operand > >& getArguments() const {
    return _args;
  }

  //! Returns the arguments passed in this call
  inline std::vector< AR_Node_Ref< AR_Operand > >& getArguments() {
    return _args;
  }

  //! Returns the return type of this call
  inline AR_Node_Ref< AR_Type > getReturnType() {
    return (*_return_value).getType();
  }

  //! Returns the returned value of this call
  inline AR_Node_Ref< AR_Internal_Variable > getReturnValue() {
    return _return_value;
  }

  //! Returns the name of the called function
  virtual ~AR_Call();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_CALL_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Call
  static std::shared_ptr< AR_Call > create(index64_t parent_bblock,
                                           s_expression e) {
    return std::static_pointer_cast< AR_Call >(
        (new AR_Call(parent_bblock, e))->shared_from_this());
  }

  //! Sets the arguments
  inline void setArguments(const std::vector< AR_Node_Ref< AR_Operand > >& o) {
    _args = o;
  }

  //! Sets the returned value
  inline void setReturnValue(AR_Node_Ref< AR_Internal_Variable > o) {
    _return_value = o;
  }

  //! Returns if the call is an asm statement
  inline bool is_inline_asm() { return _is_inline_asm; }
};

/**
 * AR_Invoke represents a function call that can throw an exception.
 */
class AR_Invoke : public AR_Statement, public ARModelEventListener {
private:
  AR_Node_Ref< AR_Call > _call;
  index64_t _normal;
  index64_t _exception;
  std::string _normal_bb_name, _except_bb_name;

private:
  AR_Invoke(index64_t parent_bblock, s_expression);

public:
  //! Returns the function call
  inline AR_Node_Ref< AR_Call > getFunctionCall() { return _call; }

  //! Returns the basic block that corresponds to the "normal" label where the
  // control flow may transfer to
  inline AR_Node_Ref< AR_Basic_Block > getNormal() {
    return AR_Node_Ref< AR_Basic_Block >(_normal);
  }

  //! Returns the basic block that corresponds to the "exception" label where
  // the control flow may transfer to
  inline AR_Node_Ref< AR_Basic_Block > getException() {
    return AR_Node_Ref< AR_Basic_Block >(_exception);
  }

  virtual ~AR_Invoke();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_INVOKE_STATEMENT_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Invoke
  static std::shared_ptr< AR_Invoke > create(index64_t parent_bblock,
                                             s_expression e) {
    return std::static_pointer_cast< AR_Invoke >(
        (new AR_Invoke(parent_bblock, e))->shared_from_this());
  }

  virtual void nodesCreated();
};

class AR_Abstract_Memory : public AR_Statement {
private:
  AR_Node_Ref< AR_Operand > _ptr;
  AR_Node_Ref< AR_Operand > _len;

private:
  AR_Abstract_Memory(index64_t parent_bblock, s_expression);

public:
  //! Returns the pointer to memory location
  inline AR_Node_Ref< AR_Operand > get_pointer() { return _ptr; }

  //! Returns the length in memory
  inline AR_Node_Ref< AR_Operand > get_length() { return _len; }

  virtual ~AR_Abstract_Memory() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_ABSTRACT_MEM_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Abstract_Memory
  static std::shared_ptr< AR_Abstract_Memory > create(index64_t parent_bblock,
                                                      s_expression e) {
    return std::static_pointer_cast< AR_Abstract_Memory >(
        (new AR_Abstract_Memory(parent_bblock, e))->shared_from_this());
  }

  //! Sets the pointer
  inline void set_pointer(AR_Node_Ref< AR_Operand > o) { _ptr = o; }

  //! Sets the length
  inline void set_length(AR_Node_Ref< AR_Operand > o) { _len = o; }
};

class AR_Abstract_Variable : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _variable;

private:
  AR_Abstract_Variable(index64_t parent_bblock, s_expression);

public:
  inline AR_Node_Ref< AR_Internal_Variable > getVariable() { return _variable; }

  virtual ~AR_Abstract_Variable() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_ABSTRACT_VAR_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Abstract_Variable
  static std::shared_ptr< AR_Abstract_Variable > create(index64_t parent_bblock,
                                                        s_expression e) {
    return std::static_pointer_cast< AR_Abstract_Variable >(
        (new AR_Abstract_Variable(parent_bblock, e))->shared_from_this());
  }

  //! Sets the variable
  inline void setVariable(AR_Node_Ref< AR_Internal_Variable > o) {
    _variable = o;
  }
};

class AR_Landing_Pad : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _exception;
  AR_Landing_Pad(index64_t parent_bblock, s_expression);

public:
  inline AR_Node_Ref< AR_Internal_Variable > getException() {
    return _exception;
  }

  virtual ~AR_Landing_Pad() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_LANDING_PAD_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Landing_Pad
  static std::shared_ptr< AR_Landing_Pad > create(index64_t parent_bblock,
                                                  s_expression e) {
    return std::static_pointer_cast< AR_Landing_Pad >(
        (new AR_Landing_Pad(parent_bblock, e))->shared_from_this());
  }

  //! Sets the exception
  inline void setException(AR_Node_Ref< AR_Internal_Variable > e) {
    _exception = e;
  }
};

class AR_Resume : public AR_Statement {
private:
  AR_Node_Ref< AR_Internal_Variable > _exception;
  AR_Resume(index64_t parent_bblock, s_expression);

public:
  inline AR_Node_Ref< AR_Internal_Variable > getException() {
    return _exception;
  }

  virtual ~AR_Resume() {}
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() { return AR_RESUME_CLASS_TYPE; }

  //! Returns the shared_ptr to AR_Resume
  static std::shared_ptr< AR_Resume > create(index64_t parent_bblock,
                                             s_expression e) {
    return std::static_pointer_cast< AR_Resume >(
        (new AR_Resume(parent_bblock, e))->shared_from_this());
  }

  //! Sets the exception
  inline void setException(AR_Node_Ref< AR_Internal_Variable > e) {
    _exception = e;
  }
};

////
// Basic blocks
////
class AR_Code;

/**
 * AR_Basic_Block represents a basic block in a CFG.
 */
class AR_Basic_Block : public AR_Node {
private:
  // body contains a series of AR_Statement(s).
  std::vector< AR_Node_Ref< AR_Statement > > _body;

  std::vector< index64_t > _next_blocks;
  std::vector< index64_t > _prev_blocks;
  std::string _name_id;
  std::unordered_set< std::string > _next_blocks_name_refs;
  std::unordered_set< std::string > _prev_blocks_name_refs;
  index64_t _containing_code;

private:
  AR_Basic_Block(index64_t parent_code, s_expression);

public:
  //! Sets the name of the basic block
  inline void setBasicBlockId(const std::string& name_id) {
    _name_id = name_id;
  }

  //! A constuctor
  AR_Basic_Block(index64_t parent_code, const std::string& name_id);

  //! Returns the body of this basic block
  inline const std::vector< AR_Node_Ref< AR_Statement > >& getStatements()
      const {
    return _body;
  }

  //! Returns the body of this basic block
  inline std::vector< AR_Node_Ref< AR_Statement > >& getStatements() {
    return _body;
  }

  //! Inserts an AR_Statement at the beginning of this body
  void insertFront(AR_Node_Ref< AR_Statement > stmt);

  //! Appends an AR_Statement at the end of this body
  void pushBack(AR_Node_Ref< AR_Statement > stmt);

  //! Appends a list of AR_Statement(s) at the end of the body
  void pushBack(const std::vector< AR_Node_Ref< AR_Statement > >& stmts);

  //! Removes the last AR_Statement of the body
  inline void popBack() { _body.pop_back(); }

  //! Retursn the last AR_Statement of the body
  inline AR_Node_Ref< AR_Statement > back() { return _body.back(); }

  //! Returns the number of AR_Statement(s) in the body
  inline unsigned int numStatements() { return _body.size(); }

  //! Replaces the old AR_Statement with the new AR_Statement
  void replace(AR_Node_Ref< AR_Statement > old_stmt,
               AR_Node_Ref< AR_Statement > new_stmt);

  /**
   * API for manipulating and browsing basic block connections
   */
  std::pair< bblock_iterator, bblock_iterator > getNextBlocks() {
    return std::
        make_pair(boost::make_transform_iterator(_next_blocks.begin(),
                                                 MkNodeRef< AR_Basic_Block >()),
                  boost::make_transform_iterator(_next_blocks.end(),
                                                 MkNodeRef<
                                                     AR_Basic_Block >()));
  }

  std::pair< bblock_iterator, bblock_iterator > getPreviousBlocks() {
    return std::
        make_pair(boost::make_transform_iterator(_prev_blocks.begin(),
                                                 MkNodeRef< AR_Basic_Block >()),
                  boost::make_transform_iterator(_prev_blocks.end(),
                                                 MkNodeRef<
                                                     AR_Basic_Block >()));
  }

  bool isNextBlock(AR_Node_Ref< AR_Basic_Block >);

  inline bool isNextBlock(const std::string& block_name) {
    std::unordered_set< std::string >::iterator it =
        _next_blocks_name_refs.find(block_name);
    return it != _next_blocks_name_refs.end();
  }

  bool isPreviousBlock(AR_Node_Ref< AR_Basic_Block >);

  inline const std::string& getNameId() { return _name_id; }
  inline AR_Node_Ref< AR_Code > getContainingCode() {
    return AR_Node_Ref< AR_Code >(_containing_code);
  }

  //! Remove statement from its code body (i.e. AR_Code)
  inline void remove(const AR_Node_Ref< AR_Statement >& stmt) {
    _body.erase(std::remove(_body.begin(), _body.end(), stmt));
  }

  /**
   * Internal API for manipulating and browsing basic block connections
   * by their reference names provided by AIR. Used by ARModel.
   */
  void addNextBasicBlock(AR_Node_Ref< AR_Basic_Block >);
  void removeNextBasicBlock(AR_Node_Ref< AR_Basic_Block >);
  void addPreviousBasicBlock(AR_Node_Ref< AR_Basic_Block >);
  void removePreviousBasicBlock(AR_Node_Ref< AR_Basic_Block >);
  inline bool isPreviousBlock(const std::string& block_name) {
    std::unordered_set< std::string >::iterator it =
        _prev_blocks_name_refs.find(block_name);
    return it != _prev_blocks_name_refs.end();
  }
  inline const std::string& getBasicBlockId() { return _name_id; }
  inline const std::unordered_set< std::string >& getNextBlockIds() {
    return _next_blocks_name_refs;
  }
  inline const std::unordered_set< std::string >& getPreviousBlockIds() {
    return _prev_blocks_name_refs;
  }

  // virtual methods for AR_Node
  virtual ~AR_Basic_Block();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() {
    return AR_BASIC_BLOCK_CLASS_TYPE;
  }

  //! Returns the shared_ptr to AR_Basic_Block
  static std::shared_ptr< AR_Basic_Block > create(index64_t parent_code,
                                                  s_expression e) {
    return std::static_pointer_cast< AR_Basic_Block >(
        (new AR_Basic_Block(parent_code, e))->shared_from_this());
  }

  static std::shared_ptr< AR_Basic_Block > create(index64_t parent_code,
                                                  const std::string& name_id) {
    return std::static_pointer_cast< AR_Basic_Block >(
        (new AR_Basic_Block(parent_code, name_id))->shared_from_this());
  }
};

class AR_Local_Variable;

/**
 * AR_Code represents the body of an AR_Function.
 */
class AR_Code : public AR_Node {
private:
  index64_t _entry_block;
  index64_t _exit_block;
  index64_t _unreachable_block;
  index64_t _ehresume_block;
  std::vector< AR_Node_Ref< AR_Basic_Block > > _blocks;
  std::vector< AR_Node_Ref< AR_Internal_Variable > > _internal_variables;
  std::unordered_map< std::string, AR_Node_Ref< AR_Basic_Block > >
      _name_id_to_bblock_cache;
  std::unordered_map< std::string, index64_t > _name_to_uid;
  bool _bblocks_connected;
  index64_t _parent_function;

private:
  void connect_basic_blocks(s_expression);
  AR_Code(index64_t parent_function, s_expression);

public:
  /*! Returns the parent function of this AR_Code.
   *  This method may return an empty AR_Function, where its UID is zero.
   *  This is the case where AR_Code is part of a global variable's initializer.
   */
  inline AR_Node_Ref< AR_Function > getContainingFunction() {
    return AR_Node_Ref< AR_Function >(_parent_function);
  }

  inline void addBasicBlock(AR_Node_Ref< AR_Basic_Block > bblock) {
    _blocks.push_back(bblock);
  }

  inline void removeBasicBlock(AR_Node_Ref< AR_Basic_Block > bblock) {
    assert(std::find(_blocks.begin(), _blocks.end(), bblock) != _blocks.end());
    if (_entry_block == bblock.getUID()) {
      _entry_block = 0;
    }
    if (_exit_block == bblock.getUID()) {
      _exit_block = 0;
    }
    if (_unreachable_block == bblock.getUID()) {
      _unreachable_block = 0;
    }
    if (_ehresume_block == bblock.getUID()) {
      _ehresume_block = 0;
    }
    _blocks.erase(std::find(_blocks.begin(), _blocks.end(), bblock));
  }

  inline AR_Node_Ref< AR_Basic_Block > getEntryBlock() {
    return AR_Node_Ref< AR_Basic_Block >(_entry_block);
  }

  inline void setEntryBlock(AR_Node_Ref< AR_Basic_Block > block) {
    _entry_block = block.getUID();
  }

  inline AR_Node_Ref< AR_Basic_Block > getExitBlock() {
    return AR_Node_Ref< AR_Basic_Block >(_exit_block);
  }

  inline void setExitBlock(AR_Node_Ref< AR_Basic_Block > block) {
    _exit_block = block.getUID();
  }

  inline AR_Node_Ref< AR_Basic_Block > getUnreachableBlock() {
    return AR_Node_Ref< AR_Basic_Block >(_unreachable_block);
  }

  inline void setUnreachableBlock(AR_Node_Ref< AR_Basic_Block > block) {
    _unreachable_block = block.getUID();
  }

  inline AR_Node_Ref< AR_Basic_Block > getEHResumeBlock() {
    return AR_Node_Ref< AR_Basic_Block >(_ehresume_block);
  }

  inline void setEHResumeBlock(AR_Node_Ref< AR_Basic_Block > block) {
    _ehresume_block = block.getUID();
  }

  AR_Node_Ref< AR_Internal_Variable > getInternalVariable(
      const std::string& name);

  inline const std::vector< AR_Node_Ref< AR_Internal_Variable > >&
  getInternalVariables() const {
    return _internal_variables;
  }

  inline std::vector< AR_Node_Ref< AR_Internal_Variable > >&
  getInternalVariables() {
    return _internal_variables;
  }

  inline const std::vector< AR_Node_Ref< AR_Basic_Block > >& getBlocks() const {
    return _blocks;
  }

  inline std::vector< AR_Node_Ref< AR_Basic_Block > >& getBlocks() {
    return _blocks;
  }

  void addInternalVariable(AR_Node_Ref< AR_Internal_Variable > ivar);
  virtual ~AR_Code();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  AR_Node_Ref< AR_Basic_Block > getBasicBlockByNameId(
      const std::string& name_id);
  index64_t getBasicBlockUIDByName(const std::string& name);
  virtual AR_CLASS_TYPE_CODE getClassType() { return AR_CODE_CLASS_TYPE; }

  //! Returns the shared_ptr to AR_Code
  static std::shared_ptr< AR_Code > create(index64_t parent_function,
                                           s_expression e) {
    return std::static_pointer_cast< AR_Code >(
        (new AR_Code(parent_function, e))->shared_from_this());
  }
};

class AR_Function;
typedef node_iterator< AR_Function, std::unordered_set< index64_t > >::type
    function_iterator;

/**
 * FunPointersInfo represents a set of points-to information on function
 * pointers.
 */
class FunPointersInfo {
public:
  // Returns all possible functions pointed by the given function pointer
  virtual std::vector< AR_Node_Ref< AR_Function > > pointedBy(
      AR_Node_Ref< AR_Internal_Variable > var) const = 0;
  virtual ~FunPointersInfo() {}
};

/**
 * AR_Function represents a function.
 */
class AR_Function : public AR_Node {
public:
  friend class AR_Bundle;

private:
  std::string _name_id;
  AR_Node_Ref< AR_Code > _function_body;
  AR_Node_Ref< AR_Function_Type > _function_type;
  std::vector< AR_Node_Ref< AR_Internal_Variable > > _parameters;
  std::vector< AR_Node_Ref< AR_Local_Variable > > _local_variables;
  AR_Node_Ref< AR_Type > _return_type;

  std::unordered_set< index64_t > _callers;
  std::unordered_set< index64_t > _callees;

private:
  const static std::string BASICBLOCK_TAG;
  AR_Function(s_expression);

  //! Computes all callee functions
  std::vector< index64_t > buildCallees(const FunPointersInfo&) const;

public:
  virtual ~AR_Function();
  virtual void print(std::ostream& out);

  //! Returns function name
  inline const std::string& getFunctionId() { return _name_id; }

  //! Returns the function body
  inline AR_Node_Ref< AR_Code > getFunctionBody() { return _function_body; }

  //! Returns the function type
  inline AR_Node_Ref< AR_Function_Type > getFunctionType() {
    return _function_type;
  }

  //! Returns the returned type; obtained through the function type
  inline AR_Node_Ref< AR_Type > getReturnType() { return _return_type; }

  //! Returns the list of formal parameters
  inline const std::vector< AR_Node_Ref< AR_Internal_Variable > >&
  getFormalParams() const {
    return _parameters;
  }

  //! Returns the list of formal parameters
  inline std::vector< AR_Node_Ref< AR_Internal_Variable > >& getFormalParams() {
    return _parameters;
  }

  //! Returns a formal parameter as a AR_Local_Variable associated by variable
  // name
  AR_Node_Ref< AR_Internal_Variable > getFormalParameter(
      const std::string& name);

  //! Returns the list of local variables
  inline const std::vector< AR_Node_Ref< AR_Local_Variable > >&
  getLocalVariables() const {
    return _local_variables;
  }

  //! Returns the list of local variables
  inline std::vector< AR_Node_Ref< AR_Local_Variable > >& getLocalVariables() {
    return _local_variables;
  }

  /*! getCallers() and getCallees() return iterators through the list of
   *  caller/callee functions, ignoring indirect and external calls
   */
  std::pair< function_iterator, function_iterator > getCallers() {
    return std::
        make_pair(boost::make_transform_iterator(_callers.begin(),
                                                 MkNodeRef< AR_Function >()),
                  boost::make_transform_iterator(_callers.end(),
                                                 MkNodeRef< AR_Function >()));
  }

  std::pair< function_iterator, function_iterator > getCallees() {
    return std::
        make_pair(boost::make_transform_iterator(_callees.begin(),
                                                 MkNodeRef< AR_Function >()),
                  boost::make_transform_iterator(_callees.end(),
                                                 MkNodeRef< AR_Function >()));
  }

  //! Returns AR_Local_Variable associated by variable name
  AR_Node_Ref< AR_Local_Variable > getLocalVariable(const std::string& name);

  void addLocalVariable(AR_Node_Ref< AR_Local_Variable > var);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() { return AR_FUNCTION_CLASS_TYPE; }

  //! Returns the shared_ptr to AR_Function
  static std::shared_ptr< AR_Function > create(s_expression e) {
    return std::static_pointer_cast< AR_Function >(
        (new AR_Function(e))->shared_from_this());
  }
};

/**
 * AR_Bundle contains all functions to be compiled in this session.
 */
class AR_Bundle : public AR_Node {
private:
  std::vector< AR_Node_Ref< AR_Function > > _functions;
  std::vector< AR_Node_Ref< AR_Global_Variable > > _globals;
  std::vector< AR_Node_Ref< AR_Function > > _entry_points;

  // metainfo
  bool _is_little_endian;
  z_number _sizeofptr;

private:
  AR_Bundle() : AR_Node() {}

public:
  static std::shared_ptr< AR_Bundle > create() {
    return std::static_pointer_cast< AR_Bundle >(
        (new AR_Bundle())->shared_from_this());
  }

  inline const std::vector< AR_Node_Ref< AR_Function > >& getFunctions() const {
    return _functions;
  }

  inline std::vector< AR_Node_Ref< AR_Function > >& getFunctions() {
    return _functions;
  }

  boost::optional< AR_Node_Ref< AR_Function > > getFunctionByNameId(
      const std::string& name) const;

  inline const std::vector< AR_Node_Ref< AR_Global_Variable > >&
  getGlobalVariables() const {
    return _globals;
  }

  inline std::vector< AR_Node_Ref< AR_Global_Variable > >&
  getGlobalVariables() {
    return _globals;
  }

  void addGlobalVariable(AR_Node_Ref< AR_Global_Variable >);
  void add_function(AR_Node_Ref< AR_Function > f) { _functions.push_back(f); }
  virtual ~AR_Bundle();
  virtual void print(std::ostream& out);
  virtual void accept(std::shared_ptr< Visitor > visitor);
  virtual AR_CLASS_TYPE_CODE getClassType() { return AR_BUNDLE_CLASS_TYPE; }

  // API to set and get metainfo
  inline void setEndianess(bool is_little_endian) {
    _is_little_endian = is_little_endian;
  }

  inline bool isLittleEndian() { return _is_little_endian; }
  inline void set_sizeofptr(z_number sizeofptr) { _sizeofptr = sizeofptr; }
  inline z_number get_sizeofptr() { return _sizeofptr; }

  /*! Build the call graph. After calling buildCallGraph(), you can use
   *  AR_Function::getCallers() and AR_Function::getCallees() to retrieve the
   *  edges.
   */
  void buildCallGraph(const FunPointersInfo&);

  /*! Clean the call graph, invalidating AR_Function::getCallers() and
   *  AR_Function::getCallees()
   */
  void cleanCallGraph();
};

/**
 * Visitor implements the visitor pattern that allow visiters to traverse the AR
 * model.
 */
class Visitor {
private:
  // The "visited" variable is a cache used to keep track of visited nodes
  // when traversing the AR tree using this Visitor class.
  // This variable is associated with two methods:
  // markVisited() and checkVisited().
  // The "visited" cache is currently used to prevent infinite loop if an
  // AR tree contains cycle dependency. A cycle can be formed by an
  // AR_Structure_Type containing an AR_Pointer_Type that points to
  // the same AR_Structure_Type. A uid is marked and checked in
  // AR_Structure_Type::accept() to prevent traversing a cycle.
  std::unordered_set< index64_t > _visited;

public:
  void markVisited(index64_t uid) { _visited.insert(uid); }
  bool checkVisited(index64_t uid) {
    return _visited.find(uid) != _visited.end();
  }

  virtual void nodeStart(AR_Node& node) {}
  virtual void nodeStart(AR_Source_Location& src_location) {}
  virtual void nodeStart(AR_Type& type) {}
  virtual void nodeStart(AR_Function_Type& func_type) {}
  virtual void nodeStart(AR_Integer_Type& int_type) {}
  virtual void nodeStart(AR_FP_Type& fp_type) {}
  virtual void nodeStart(AR_Void_Type& void_type) {}
  virtual void nodeStart(AR_Pointer_Type& ptr_type) {}
  virtual void nodeStart(AR_Structure_Type& struct_type) {}
  virtual void nodeStart(AR_Array_Type& arr_type) {}
  virtual void nodeStart(AR_Constant& cst) {}
  virtual void nodeStart(AR_Integer_Constant& int_cst) {}
  virtual void nodeStart(AR_FP_Constant& fp_cst) {}
  virtual void nodeStart(AR_Null_Constant& null_cst) {}
  virtual void nodeStart(AR_Undefined_Constant& u) {}
  virtual void nodeStart(AR_Function_Addr_Constant& addr_cst) {}
  virtual void nodeStart(AR_Internal_Variable& var) {}
  virtual void nodeStart(AR_Operand& operand) {}
  virtual void nodeStart(AR_Cst_Operand& cst_operand) {}
  virtual void nodeStart(AR_Var_Operand& var_operand) {}
  virtual void nodeStart(AR_Statement& stmt) {}
  virtual void nodeStart(AR_Integer_Comparison& icmp) {}
  virtual void nodeStart(AR_FP_Comparison& fcmp) {}
  virtual void nodeStart(AR_Assignment& assign) {}
  virtual void nodeStart(AR_Arith_Op& arith_op) {}
  virtual void nodeStart(AR_Conv_Op& conv_op) {}
  virtual void nodeStart(AR_Bitwise_Op& bitwise_op) {}
  virtual void nodeStart(AR_FP_Op& fp_op) {}
  virtual void nodeStart(AR_Store& store) {}
  virtual void nodeStart(AR_Load& load) {}
  virtual void nodeStart(AR_Store_Element& store_element) {}
  virtual void nodeStart(AR_Load_Element& load_element) {}
  virtual void nodeStart(AR_NOP& nop) {}
  virtual void nodeStart(AR_Unreachable& unreachable) {}
  virtual void nodeStart(AR_Landing_Pad& landingpad) {}
  virtual void nodeStart(AR_Resume& resume) {}
  virtual void nodeStart(AR_Return_Value& ret_val) {}
  virtual void nodeStart(AR_MemCpy& memcpy) {}
  virtual void nodeStart(AR_MemMove& memmv) {}
  virtual void nodeStart(AR_MemSet& memset) {}
  virtual void nodeStart(AR_Pointer_Shift& ptr_shift) {}
  virtual void nodeStart(AR_Call& call) {}
  virtual void nodeStart(AR_Invoke& meth_call) {}
  virtual void nodeStart(AR_Abstract_Variable& ab_var) {}
  virtual void nodeStart(AR_Abstract_Memory& ab_mem) {}
  virtual void nodeStart(AR_VA_Start& va_start) {}
  virtual void nodeStart(AR_VA_End& va_end) {}
  virtual void nodeStart(AR_VA_Arg& va_arg) {}
  virtual void nodeStart(AR_VA_Copy& va_copy) {}
  virtual void nodeStart(AR_Variable& var) {}
  virtual void nodeStart(AR_Local_Variable& var) {}
  virtual void nodeStart(AR_Global_Variable& var) {}
  virtual void nodeStart(AR_Var_Addr_Constant& addr_cst) {}
  virtual void nodeStart(AR_Bundle& bundle) {}
  virtual void nodeStart(AR_Basic_Block& bblock) {}
  virtual void nodeStart(AR_Code& code) {}
  virtual void nodeStart(AR_Function& func) {}

  virtual void nodeEnd(AR_Node& node) {}
  virtual void nodeEnd(AR_Source_Location& src_location) {}
  virtual void nodeEnd(AR_Type& type) {}
  virtual void nodeEnd(AR_Function_Type& func_type) {}
  virtual void nodeEnd(AR_Integer_Type& int_type) {}
  virtual void nodeEnd(AR_FP_Type& fp_type) {}
  virtual void nodeEnd(AR_Void_Type& void_type) {}
  virtual void nodeEnd(AR_Pointer_Type& ptr_type) {}
  virtual void nodeEnd(AR_Structure_Type& struct_type) {}
  virtual void nodeEnd(AR_Array_Type& arr_type) {}
  virtual void nodeEnd(AR_Constant& cst) {}
  virtual void nodeEnd(AR_Integer_Constant& int_cst) {}
  virtual void nodeEnd(AR_FP_Constant& fp_cst) {}
  virtual void nodeEnd(AR_Null_Constant& null_cst) {}
  virtual void nodeEnd(AR_Internal_Variable& var) {}
  virtual void nodeEnd(AR_Operand& operand) {}
  virtual void nodeEnd(AR_Cst_Operand& cst_operand) {}
  virtual void nodeEnd(AR_Var_Operand& var_operand) {}
  virtual void nodeEnd(AR_Statement& stmt) {}
  virtual void nodeEnd(AR_Integer_Comparison& icmp) {}
  virtual void nodeEnd(AR_FP_Comparison& fcmp) {}
  virtual void nodeEnd(AR_Assignment& assign) {}
  virtual void nodeEnd(AR_Arith_Op& arith_op) {}
  virtual void nodeEnd(AR_Conv_Op& conv_op) {}
  virtual void nodeEnd(AR_Bitwise_Op& bitwise_op) {}
  virtual void nodeEnd(AR_FP_Op& fp_op) {}
  virtual void nodeEnd(AR_Store& store) {}
  virtual void nodeEnd(AR_Load& load) {}
  virtual void nodeEnd(AR_Store_Element& store_element) {}
  virtual void nodeEnd(AR_Load_Element& load_element) {}
  virtual void nodeEnd(AR_NOP& nop) {}
  virtual void nodeEnd(AR_Unreachable& unreachable) {}
  virtual void nodeEnd(AR_Landing_Pad& landingpad) {}
  virtual void nodeEnd(AR_Resume& resume) {}
  virtual void nodeEnd(AR_Return_Value& ret_val) {}
  virtual void nodeEnd(AR_MemCpy& memcpy) {}
  virtual void nodeEnd(AR_MemMove& memmv) {}
  virtual void nodeEnd(AR_MemSet& memset) {}
  virtual void nodeEnd(AR_Pointer_Shift& ptr_shift) {}
  virtual void nodeEnd(AR_VA_Start& va_start) {}
  virtual void nodeEnd(AR_VA_End& va_end) {}
  virtual void nodeEnd(AR_VA_Arg& va_arg) {}
  virtual void nodeEnd(AR_VA_Copy& va_copy) {}

  virtual void nodeEnd(AR_Basic_Block& bblock) {}
  virtual void nodeEnd(AR_Code& code) {}
  virtual void nodeEnd(AR_Function& func) {}
  virtual void nodeEnd(AR_Call& call) {}
  virtual void nodeEnd(AR_Invoke& meth_call) {}
  virtual void nodeEnd(AR_Abstract_Variable& ab_var) {}
  virtual void nodeEnd(AR_Abstract_Memory& ab_mem) {}
  virtual void nodeEnd(AR_Function_Addr_Constant& addr_cst) {}
  virtual void nodeEnd(AR_Variable& var) {}
  virtual void nodeEnd(AR_Local_Variable& var) {}
  virtual void nodeEnd(AR_Global_Variable& var) {}
  virtual void nodeEnd(AR_Var_Addr_Constant& addr_cst) {}
  virtual void nodeEnd(AR_Bundle& bundle) {}

  virtual void nodeEnd(AR_Undefined_Constant& u) {}

  virtual void attributeStart(AR_Node& node, std::string attrName) {}
  virtual void attributeEnd(AR_Node& node, std::string attrName) {}
  virtual void listStart() {}
  virtual void listContinued() {}
  virtual void listEnd() {}
  virtual ~Visitor() {}
};

class ARModel {
public:
  friend class AR_Node;
  template < class T >
  friend class AR_Node_Ref;

private:
  static ARModel* _instance;
  // References from the input
  std::vector< index64_t > _ordered_refs;
  std::unordered_map< index64_t, boost::optional< s_expression > >
      _input_ref_to_sexpr;
  std::unordered_map< index64_t, index64_t > _input_ref_to_ar_map;
  std::unordered_map< index64_t, std::string >
      _file_map; // maps AIR ref to the actual file path

  // ARModel event listeners
  std::unordered_map< ARModelEventListener::Event,
                      std::vector< index64_t >,
                      ARModelEventListener::EventHash > _listeners;

  std::unordered_map< index64_t, std::shared_ptr< AR_Node > > _uid_to_ARNode;
  std::list< AR_Node_Ref< AR_Node > > _ar_prototypes;
  AR_Node_Ref< AR_Bundle > _bundle;
  std::unordered_map< std::string, index64_t > _name_id_to_func;
  AR_Node_Ref< AR_Code > _current_building_scope;
  AR_Node_Ref< AR_Function > _current_building_function;
  bool _complete;

  static std::string METAINFO;
  static std::string FUNCTION;
  static std::string ENDIANESS;
  static std::string LITTLEENDIAN;
  static std::string SIZEOFPTR;
  static std::string TYPE;

public:
  ARModel() : _complete(false) {}
  ~ARModel() {}

  // Begin methods handling the input
  void generate(std::istream& is);

  inline index64_t getARfromInputRef(index64_t id) {
    return _input_ref_to_ar_map.find(id) != _input_ref_to_ar_map.end()
               ? _input_ref_to_ar_map[id]
               : 0;
  }

  inline std::string get_filepath(index64_t id) { return _file_map[id]; }
  inline std::unordered_map< index64_t, std::string >& getFiles() {
    return _file_map;
  }
  inline s_expression getSEXPRfromInputRef(index64_t id) {
    return *_input_ref_to_sexpr[id];
  }

  inline void registerARtoInputRef(index64_t uid, index64_t id) {
    _input_ref_to_ar_map[id] = uid;
    _ar_prototypes.push_back(AR_Node_Ref< AR_Node >(uid));
  }

  inline std::list< AR_Node_Ref< AR_Node > >& getARFlyweights() {
    return _ar_prototypes;
  }
  // End methods handling the input

  void print(std::ostream& out);
  inline std::shared_ptr< AR_Node > getARNode(index64_t uid) {
    return _uid_to_ARNode[uid];
  }

  inline index64_t getTotalNumberARNodes() { return _uid_to_ARNode.size(); }

  inline index64_t getFunctionUIDByNameId(const std::string& name_id) {
    if (_name_id_to_func.find(name_id) != _name_id_to_func.end()) {
      return _name_id_to_func[name_id];
    }
    return 0;
  }

  inline void registerARNode(index64_t uid, std::shared_ptr< AR_Node > node) {
    assert(node);
    _uid_to_ARNode.insert(std::make_pair(uid, node));
  }

  inline void registerFunction(AR_Node_Ref< AR_Function > func) {
    if (_name_id_to_func.find((*func).getFunctionId()) !=
        _name_id_to_func.end()) {
      throw error("function " + (*func).getFunctionId() + " already exists");
    } else {
      _name_id_to_func[(*func).getFunctionId()] = func.getUID();
    }
  }

  inline AR_Node_Ref< AR_Bundle > getBundle() { return _bundle; }
  inline void setCurrentBuildingScope(AR_Node_Ref< AR_Code > scope) {
    _current_building_scope = scope;
  }

  inline AR_Node_Ref< AR_Code > getCurrentBuildingScope() {
    return _current_building_scope;
  }

  inline void setBundle(AR_Node_Ref< AR_Bundle > bundle) { _bundle = bundle; }

  inline void setCurrentBuildingFunction(AR_Node_Ref< AR_Function > f) {
    _current_building_function = f;
  }

  inline AR_Node_Ref< AR_Function > getCurrentBuildingFunction() {
    return _current_building_function;
  }

  void accept(std::shared_ptr< Visitor > visitor);
  inline void addARPrototype(AR_Node_Ref< AR_Node > node_ref) {
    _ar_prototypes.push_back(node_ref);
  }

  bool isModelComplete() { return _complete; }

  void add_listener(ARModelEventListener::Event topic, index64_t uid) {
    _listeners[topic].push_back(uid);
  }

  static ARModel* Instance() { return _instance; }

private:
  void process_meta_info(s_expression);
  void process_global_defs();
  void dump_ref_counts();
};

template < typename T >
AR_Node_Ref< T >::AR_Node_Ref(index64_t uid)
    : _uid(uid) {
  ReferenceCounter::Get()->increment_ct(uid);
}

template < typename T >
AR_Node_Ref< T >::AR_Node_Ref(AR_Node_Ref< T > const& node_ref) {
  _uid = node_ref.getUID();
  ReferenceCounter::Get()->increment_ct(_uid);
}

template < typename T >
AR_Node_Ref< T >& AR_Node_Ref< T >::operator=(const AR_Node_Ref< T >& rhs) {
  if (_uid != rhs.getUID()) {
    swap(rhs.getUID());
  }
  return *this;
}

template < typename T >
void AR_Node_Ref< T >::setUID(index64_t new_uid) {
  if (_uid == new_uid)
    return;

  swap(new_uid);
}

template < typename T >
T& AR_Node_Ref< T >::operator*() const {
  std::shared_ptr< T > ptr =
      std::static_pointer_cast< T >(ARModel::Instance()->getARNode(_uid));
  assert(ptr);
  return *(ptr.get());
}

template < typename T >
T* AR_Node_Ref< T >::operator->() const {
  std::shared_ptr< T > ptr =
      std::static_pointer_cast< T >(ARModel::Instance()->getARNode(_uid));
  assert(ptr);
  return ptr.get();
}

template < typename T >
AR_Node_Ref< T >::~AR_Node_Ref() {
  if (_uid > 0) {
    if (ReferenceCounter::Get()->get_ct(_uid) > 0) {
      ReferenceCounter::Get()->decrement_ct(_uid);
    }
    if (ReferenceCounter::Get()->get_ct(_uid) == 0) {
      std::unordered_map< index64_t, std::shared_ptr< AR_Node > >::iterator p =
          ARModel::Instance()->_uid_to_ARNode.find(_uid);
      if (p != ARModel::Instance()->_uid_to_ARNode.end()) {
#ifdef DEBUG
        std::string class_type =
            ReferenceCounter::Get()->str(p->second->getClassType());
        std::cerr << "Deleting AR Node #" << _uid << "(" << class_type << ")"
                  << std::endl;
#endif
        ARModel::Instance()->_uid_to_ARNode.erase(p);
#ifdef DEBUG
        std::cerr << "AR Node #" << _uid << " (" << class_type
                  << ") has been garbage-collected." << std::endl;
#endif
      }
    }
  }
}

template < typename T >
void AR_Node_Ref< T >::swap(index64_t new_uid) {
  index64_t old_uid = _uid;
  _uid = new_uid;
  ReferenceCounter::Get()->increment_ct(_uid);
  if (old_uid > 0) {
    ReferenceCounter::Get()->decrement_ct(old_uid);
  }
}

template < typename T >
std::ostream& operator<<(std::ostream& out, const AR_Node_Ref< T >& ref) {
  (*ref).print(out);
  return out;
}

/**
 * ARFactory consists of factory methods for creating AR nodes.
 */
class ARFactory {
private:
  // AR Data Types
  static const std::string BOOL_TYPE;
  static const std::string INTEGER_TYPE;
  static const std::string FLOAT_TYPE;
  static const std::string FUNCTION_TYPE;
  static const std::string POINTER_TYPE;
  static const std::string ARRAY_TYPE;
  static const std::string STRUCT_TYPE;
  static const std::string VOID_TYPE;
  static const std::string OPAQUE_TYPE;

  // XAR Data Types
  static const std::string LABEL_TYPE;

  // Conversion operators
  static const std::string TRUNC_OP;
  static const std::string Z_EXT_OP;
  static const std::string S_EXT_OP;
  static const std::string FP_TRUNC_OP;
  static const std::string FP_EXT_OP;
  static const std::string FP_TO_UI_OP;
  static const std::string FP_TO_SI_OP;
  static const std::string UI_TO_FP_OP;
  static const std::string SI_TO_FP_OP;
  static const std::string PTR_TO_INT_OP;
  static const std::string INT_TO_PTR_OP;
  static const std::string BITCAST_OP;

  // Bitwise Operations
  static const std::string BITWISE_SHL;
  static const std::string BITWISE_LSHR;
  static const std::string BITWISE_ASHR;
  static const std::string BITWISE_AND;
  static const std::string BITWISE_OR;
  static const std::string BITWISE_XOR;

  // FP operations
  static const std::string FADD;
  static const std::string FSUB;
  static const std::string FMUL;
  static const std::string FDIV;
  static const std::string FREM;

  // Arithmetic operators
  static const std::string ADD_OP;
  static const std::string SUB_OP;
  static const std::string MUL_OP;
  static const std::string UDIV_OP;
  static const std::string SDIV_OP;
  static const std::string UREM_OP;
  static const std::string SREM_OP;

  // Register operators
  static const std::string ASSIGN;

  // Register operators for aggregate types
  static const std::string INSERT_VALUE;
  static const std::string EXTRACT_VALUE;

  // Assertion operators
  static const std::string ASSERT;

  // Memory statements
  static const std::string LOAD_OP;
  static const std::string STORE_OP;

  // Return statement
  static const std::string RET_OP;

  // xar operators
  static const std::string XAR_BR_OP;
  static const std::string XAR_GETELEMETRYPOINTER_OP;
  static const std::string XAR_STORE_OP;
  static const std::string XAR_LOAD_OP;
  static const std::string XAR_PHI_OP;
  static const std::string XAR_ICMP_OP;
  static const std::string XAR_FCMP_OP;

  // function call
  static const std::string CALL;

  // Exception handling
  static const std::string LANDINGPAD;
  static const std::string RESUME;

  // misc. statements
  static const std::string UNREACHABLE;
  static const std::string RANDOMIZE;

  // Operand Types
  static const std::string CONSTANT_TYPE;
  static const std::string VAR_TYPE;
  static const std::string GLOBALVAR_TYPE;
  static const std::string NAMEDVAR_TYPE;
  static const std::string IDX_TYPE;

  // Constant Types
  static const std::string CST_BOOL;
  static const std::string CST_INTEGER;
  static const std::string CST_FLOAT;
  static const std::string CST_AGGRZERO;
  static const std::string GLOBAL_VAR_REF;
  static const std::string LOCAL_VAR_REF;
  static const std::string CST_ARRAY;
  static const std::string CST_STRUCT;
  static const std::string CST_POINTER_NULL;
  static const std::string CST_FUNCTION_PTR;
  static const std::string CST_UNDEFINED;

  // Pointer operators
  static const std::string ALLOCA_OP;

  // Metadata
  static const std::string LINE;

private:
  ARFactory() {}

  // Global Constant AR_Type instances
  static AR_Node_Ref< AR_Void_Type > VOID_TYPE_AR_NODE;
  static AR_Node_Ref< AR_Undefined_Constant > VOID_UNDEFINED_CONSTANT_AR_NODE;

public:
  // Factory methods processing s-expressions
  static AR_Node_Ref< AR_Type > createType(index64_t);
  static AR_Node_Ref< AR_Constant > createConstant(s_expression);
  static AR_Node_Ref< AR_Global_Variable > createGlobalVariable(index64_t);
  static AR_Node_Ref< AR_Function > createFunction(s_expression);
  static AR_Node_Ref< AR_Operand > createOperand(s_expression);
  static AR_Node_Ref< AR_Source_Location > createSourceLocation(s_expression);
  static AR_Node_Ref< AR_Code > createCode(index64_t parent_function,
                                           s_expression);
  static AR_Node_Ref< AR_Basic_Block > createBasicBlock(index64_t parent_code,
                                                        s_expression);
  static std::vector< AR_Node_Ref< AR_Statement > > createStatements(
      index64_t parent_bblock, s_expression);
  static AR_Node_Ref< AR_Internal_Variable > createInternalVariable(
      s_expression);

  // Methods
  static AR_Node_Ref< AR_Operand > createConstantInteger(
      z_number, AR_Node_Ref< AR_Integer_Type >);
  static AR_Node_Ref< AR_Operand > createVarOperand(
      AR_Node_Ref< AR_Internal_Variable >);
};

/**
 * Options allow the user to give parameters to an ARBOS Pass
 */

class option_error : public error {
private:
  option_error();

public:
  option_error(std::string msg) : error(msg) {}

  std::string message() { return _msg; }

}; // class option_error

class OptionParser;

class OptionBase {
  friend class OptionParser;

protected:
  typedef boost::program_options::option_description option_description;

protected:
  boost::shared_ptr< option_description > _opt_desc;
  const OptionParser* _parser;

protected:
  OptionBase() = delete;
  OptionBase(const OptionBase&) = delete;

  OptionBase(boost::shared_ptr< option_description > opt_desc)
      : _opt_desc(opt_desc), _parser(nullptr) {
    this->registerOption();
  }

  virtual ~OptionBase() { this->unregisterOption(); }

private:
  // register/unregister the option in the global OptionRegistry
  void registerOption();
  void unregisterOption();

protected:
  void setParser(const OptionParser& parser) { _parser = &parser; }

  boost::shared_ptr< option_description > getOptionDescription() const {
    return _opt_desc;
  }

public:
  const std::string& getLongName() const { return _opt_desc->long_name(); }

  const std::string& getDescription() const { return _opt_desc->description(); }

}; // class OptionBase

class OptionParser {
private:
  typedef boost::program_options::variables_map variables_map;
  typedef boost::program_options::option_description option_description;
  typedef boost::program_options::options_description options_description;

private:
  variables_map _var_map;
  options_description _options;

public:
  OptionParser(const std::string& label = "Optional arguments")
      : _options(label) {}

  bool isDefined(const std::string& opt_long_name) const {
    for (std::vector< boost::shared_ptr< option_description > >::const_iterator
             it = _options.options().begin();
         it != _options.options().end();
         ++it) {
      if (opt_long_name == (*it)->long_name()) {
        return true;
      }
    }
    return false;
  }

  bool add(OptionBase& option) {
    option.setParser(*this);

    if (!isDefined(option.getLongName())) {
      _options.add(option.getOptionDescription());
      return true;
    } else {
      return false;
    }
  }

  void parse(int argc, char** argv) {
    namespace po = boost::program_options;

    try {
      po::store(po::command_line_parser(argc, argv)
                    .options(_options)
                    .style(po::command_line_style::unix_style |
                           po::command_line_style::allow_long_disguise)
                    .run(),
                _var_map);
      po::notify(_var_map);
    } catch (po::error& e) {
      throw option_error(e.what());
    }
  }

  void parse(const std::vector< std::string >& args) {
    namespace po = boost::program_options;

    try {
      po::store(po::command_line_parser(args)
                    .options(_options)
                    .style(po::command_line_style::unix_style |
                           po::command_line_style::allow_long_disguise)
                    .run(),
                _var_map);
      po::notify(_var_map);
    } catch (po::error& e) {
      throw option_error(e.what());
    }
  }

  bool hasValue(const std::string& opt_long_name) const {
    return _var_map.count(opt_long_name);
  }

  template < typename T >
  const T& getValue(const std::string& opt_long_name) const {
    if (!_var_map.count(opt_long_name)) {
      throw option_error("parser does not have a value for option " +
                         opt_long_name);
    }

    try {
      return _var_map[opt_long_name].as< T >();
    } catch (boost::bad_any_cast& e) {
      throw option_error(e.what());
    }
  }

  friend std::ostream& operator<<(std::ostream& o, const OptionParser& parser) {
    o << parser._options;
    return o;
  }

}; // class OptionParser

template < typename T >
class Option : public OptionBase {
public:
  Option(const std::string& name, const std::string& description)
      : OptionBase(boost::make_shared<
                   option_description >(name.c_str(),
                                        boost::program_options::value< T >(),
                                        description.c_str())) {}

  Option(const std::string& name,
         const std::string& description,
         const T& default_val)
      : OptionBase(boost::make_shared<
                   option_description >(name.c_str(),
                                        boost::program_options::value< T >()
                                            ->default_value(default_val),
                                        description.c_str())) {}

  bool hasValue() const {
    assert(_parser != nullptr);
    return _parser->hasValue(this->getLongName());
  }

  const T& getValue() const {
    assert(_parser != nullptr);
    return _parser->getValue< T >(this->getLongName());
  }

  // shortcuts
  operator bool() const { return this->hasValue(); }
  operator T() const { return this->getValue(); }

  friend std::ostream& operator<<(std::ostream& o, const Option< T >& opt) {
    if (opt.hasValue()) {
      o << opt.getValue();
    } else {
      o << "[undefined]";
    }
    return o;
  }

}; // class Option< T >

// Specialization for Option< bool >
template <>
class Option< bool > : public OptionBase {
public:
  Option(const std::string& name, const std::string& description)
      : OptionBase(
            boost::make_shared<
                option_description >(name.c_str(),
                                     new boost::program_options::untyped_value(
                                         true),
                                     description.c_str())) {}

  bool hasValue() const {
    assert(_parser != nullptr);
    return _parser->hasValue(this->getLongName());
  }

  bool getValue() const { return this->hasValue(); }

  // shortcut
  operator bool() const { return this->hasValue(); }

  friend std::ostream& operator<<(std::ostream& o, const Option< bool >& opt) {
    o << opt.hasValue();
    return o;
  }

}; // class Option< bool >

/*
 * Singleton holding all declared options
 */
class OptionRegistry {
private:
  static OptionRegistry* _instance;

public:
  typedef boost::indirect_iterator< std::vector< OptionBase* >::iterator >
      iterator;
  typedef boost::indirect_iterator< std::vector< OptionBase* >::const_iterator >
      const_iterator;

private:
  std::vector< OptionBase* > _options;

private:
  OptionRegistry() {}

  OptionRegistry(const OptionRegistry&) = delete;

  ~OptionRegistry() {}

public:
  // the following methods are directly called in the OptionBase constructor
  // and destructor
  void add(OptionBase& option) { _options.push_back(&option); }
  void remove(OptionBase& option) {
    std::remove(_options.begin(), _options.end(), &option);
  }

  iterator begin() { return _options.begin(); }
  const_iterator begin() const { return _options.begin(); }

  iterator end() { return _options.end(); }
  const_iterator end() const { return _options.end(); }

  static OptionRegistry* Get() {
    if (_instance == nullptr) {
      _instance = new OptionRegistry();
    }

    return _instance;
  }

}; // class OptionRegistry

/**
 * Pass is served as an application extension point in the plugin framework. A
 * pass gets to traverse the AR model and perform analysis.
 */
class Pass {
private:
  std::string shortname;
  std::string description;

public:
  virtual ~Pass() {}
  virtual void execute(AR_Node_Ref< AR_Bundle > bundle) = 0;
  const std::string& getShortName() const { return shortname; }
  const std::string& getDescription() const { return description; }

protected:
  Pass(const std::string& s, const std::string& d)
      : shortname(s), description(d) {}
};

} // namepace arbos

#endif // ARBOS_AR_HPP
