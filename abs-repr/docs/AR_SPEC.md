# AR Specification

## Introduction

This document illustrates the Abstract Representation (AR) in S-expressions.

## AR Specification
Serialized versions of the AR are encoded in an extended S-expression-like format that permits preservation of sharing and recursion. It is strictly more expressive than XML (i.e., its underlying topology is that of a directed graph rather than a tree).

### Atoms

#### Strings

All strings are prefixed by $. Punctuation and white space must be escaped by being prefixed with '\', e.g.:

```
$fred
$this\ has\ spaces\ in\ it
$Hello\,\ World\!
$yada\09\20\23\24\25\24\24\5E\25\5E\24\5E\25\5E\26\25\2A\26\20yada\0A for "yada\t #$%$$^%^$^%^&%*& yada\n"
```

#### Integers

Integers are prefixed with #, e.g.

```
#1
#-345
```

#### Floating point

```
^0.88
```

#### Q numbers

Q numbers are prefixed with %, e.g.

```
%-4/7
```

#### UIDs

Each AR node has an unique identification represented as unsigned 64bit integer. UIDs are prefixed with !, e.g.

```
!33
```

### S-expressions

#### Types

The AR output lists all the types referenced in the entire program prior to the function definitions. Each type is associated with a UID. The following illustrates the general s-expression format for types.

```
(![uid] ($type ($[type_name]) ([...])))
```

##### Integer type

```
($int (#8) ($realsize (#8)) ($storesize (#1)))
```

represents an integer type of size 8 bits.

##### Floating point type

```
($float ($double) ($realsize (#64)) ($storesize (#8)))
```

##### Array type

```
($array ($len (#10)) ($ty (!4)))))), where #10 represents the length of the array; ($ty (!4)) represents the cell type.
```

##### Pointer type

```
($ptr ($ty (!18)) ($realsize (#64)) ($storesize (#8))), where ($ty (!18)) represents the referenced type
```

##### Struct type

The struct type includes information of the struct layout, alignment, and the total padded size in bits. The format of the layout specifies a sequence of (#padded_offset (!type-uid-ref)) for each field of the struct.

```
($struct ($layout (#0 (!16))) ($align (#64)) ($realsize (#128)) ($storesize (#16)))
```

##### Function type

```
($function ($return_ty (!34)) ($params_ty (!48) (!25)) ($isVarArg ($False)))
```

##### Void type

```
($void ($realsize (#0)) ($storesize (#0)))
```

#### Constants

The following illustrates the general s-expression format for constants.

```
($cst ($[constant-type]) ([...]))
```

##### Integer constant

```
($constantint ($val (#0)) ($ty (!2)))
```

##### Floating point constant

```
($constantfp ($val (^0.88)) ($ty (!4)))
```

##### Null constant

```
($constantpointernull ($ty (!22)))
```

##### Undefined

```
(!23 ($def ($undefined) (!11)))
```

##### Range

```
($range
  ($values
    ($value (#0) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
    ($value (#64) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
    ($value (#128) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
    ($value (#192) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
    ($value (#256) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
    ($value (#320) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
    ($value (#384) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
    ($value (#448) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
    ($value (#512) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
    ($value (#576) ($cst ($constantfp ($val (^0x0p0)) ($ty (!4)))))
  ($ty (!7))))
```

##### Variable Address

```
($globalvariableref ($name ($a)) ($gv (!2)) ($ty (!7))), or
($localvariableref ($name ($a)) ($ty (!11)))
```

#### Variables

##### Global variable

```
($gv ($name ($)) ($ty (!)) ($align (#)) ($code) ($debug ($srcloc)))
```

##### Global variable reference

```
($globalvariableref ($name ($a)) ($gv (!2)) ($ty (!7)))
```

##### Local variable

```
($local_var ($var ($name ($str)) ($ty (!2)) ($align (#4) )))
```

##### Local variable reference

```
($localvariableref ($name ($a)) ($ty (!7)))
```

##### Internal variable

```
($var ($name ($tmp)) ($ty (!11)))
```


#### Operands

##### Variable operand

```
($var ($name ($i.0)) ($ty (!2)))), where ($i\2E0) (that is "i.0") is the name of the variable and (!2) is the reference uid of its type.
```


##### Constant operand

```
($cst ($constantint ($val (#9)) ($ty (!9)))
```


#### Statements

##### Assignment

```
($assign ($lhs ($var ($name ($i.o)) ($ty (!7)))) ($rhs ($var ($name ($tmp5)) ($ty (!7)))) ($debug ($srcloc))), where \
  ($lhs ($var ($name ($i.0)) ($ty (!7)))) is the result, while ($rhs ($var ($name ($tmp5)) ($ty (!7)))) \
  is the operand of the right-hand-side.
```

##### Asssertion

```
($assert ($kind ($i))
         ($cmp ($cond ($true)) ($pred ($eq))
         ($ops ($var ($name ($eh_select)) ($ty (!24))) ($var ($name ($eh_typeid)) ($ty (!24)))))
         ($debug ($srcloc ($line (#48)) ($file (!27)))))
```

##### Arithmic operation

```
($mul
    ($lhs ($var ($name ($__v:43)) ($ty (!16))))
    ($rhs ($cst ($constantint ($val (#3)) ($ty (!16))))
          ($cst ($constantint ($val (#0)) ($ty (!16)))))
    ($debug ($srcloc ($line (#-1)) ($file (!17))))), where
  ($var ($name ($__v:43)) ($ty (!16))) is the result, while \
  ($cst ($constantint ($val (#3)) ($ty (!16)))) and \
  ($cst ($constantint ($val (#0)) ($ty (!16)))) are operands on the right-hand-side.
```

##### AR_FP_Op

```
($fmul
  ($lhs ($var ($name ($tmp3)) ($ty (!4))))
  ($rhs ($var ($name ($tmp1)) ($ty (!4))) ($var ($name ($tmp2)) ($ty (!4))))
  ($debug ($srcloc ($line (#9)) ($file (!6)))))
```

##### AR_Bitwise_Op

```
($or
  ($lhs ($var ($name ($tmp)) ($ty (!2))))
  ($rhs ($cst ($constantint ($val (#4)) ($ty (!2)))) ($cst ($constantint ($val (#5)) ($ty (!2)))))
  ($debug ($srcloc ($line (#3)) ($file (!6)))))
```

##### AR_Conv_Op

```
($sitofp
  ($lhs ($var ($name ($tmp1)) ($ty (!4))))
  ($rhs ($var ($name ($tmp)) ($ty (!11))))
  ($debug ($srcloc ($line (#9)) ($file (!6)))))
```

##### AR_Integer_Comparison

```
($cmp ($cond ($true)) ($pred ($sle)) ($ops ($var ($name ($i\2E0)) ($ty (!11))) ($cst ($constantint ($val (#22)) ($ty (!11))))))
```

##### AR_FP_Comparison

```
($cmp ($cond ($true)) ($pred ($sle)) ($ops ($var ($name ($i\2E0)) ($ty (!11))) ($cst ($constantint ($val (#22)) ($ty (!11))))))
```

##### AR_MemSet

```
       ($memset
          ($ret ($var ($name ($\5C5Fv)) ($ty (!12))))
          ($dest ($var ($name ($\5C5F\5C5Fv)) ($ty (!9))))
          ($val ($cst ($constantint ($val (#13)) ($ty (!10)))))
          ($len ($cst ($constantint ($val (#14)) ($ty (!15)))))
          ($align (#16))
          ($isvolatile ($cst ($constantint ($val (#17)) ($ty (!18)))))
          ($debug ($srcloc ($line (#7)) ($file (!6)))))
```

##### AR_MemCpy
```
     ($memcpy
     ($ret ())
     ($dest ())
     ($src ())
     ($len ())
     ($align (#16))
     ($isvolatile ($cst ($constantint ($val (#17)) ($ty (!18)))))
     ($debug ($srcloc ($line (#7)) ($file (!6)))))
```

##### AR_MemMove
```
     ($memmove
     ($ret ())
     ($dest ())
     ($src ())
     ($len ())
     ($align (#16))
     ($isvolatile ($cst ($constantint ($val (#17)) ($ty (!18)))))
     ($debug ($srcloc ($line (#7)) ($file (!6)))))
```

##### AR_VA_Start

```
($va_start ($var ($name ($vl12)) ($ty (!6))) ($debug ($srcloc ($line (#14)) ($file (!14)))))
```

##### AR_VA_Copy

```
($va_copy ($dest ($var ($name ($vl_count35)) ($ty (!6)))) ($src ($var ($name ($vl46)) ($ty (!6)))) ($debug ($srcloc ($line (#17)) ($file (!14)))))
```

##### AR_VA_End

```
($va_end ($var ($name ($vl_count1213)) ($ty (!6))) ($debug ($srcloc ($line (#22)) ($file (!14)))))
```

#####  AR_VA_Arg

```
($va_arg ($ret) ($ptr) ($debug))
```

##### AR_Pointer_Shift

```
($ptrshift
  ($lhs ($var ($name ($__v:1)) ($ty (!19))))
  ($rhs ($cst ($globalvariableref ($name ($_ZTI1A)) ($gv (!3)) ($ty (!14))))
        ($cst ($constantint ($val (#0)) ($ty (!20)))))
  ($debug ($srcloc ($line (#-1)) ($file (!13)))))
```

##### AR_Allocate

```
($allocate 
  ($dest ($cst ($localvariableref ($name ($foo.vla)) ($ty (!13))))) 
  ($alloca_ty (!10)) 
  ($array_size ($var ($name ($foo._2)) ($ty (!7)))) 
  ($debug ($srcloc ($line (#3)) ($col (#5)) ($file (!11)))))
```

##### AR_Store

```
($store ($lhs ($var ($name ($d)) ($ty (!4)))) ($rhs ($constantfp ($val (^0x1.c28f5c28f5c29p-1)) ($ty (!4)))) ($align (#4)) ($debug ($srcloc ($line (#3)) ($file (!5)))))
```

##### AR_Load

```
($load ($lhs ($var ($name ($tmp2)) ($ty (!4)))) ($rhs ($cst ($constantint ($val (#17)) ($ty (!3))))) ($align (#4)) ($debug ($srcloc ($line (#9)) ($file (!6)))))
```

##### AR_Insert_Element

```
($insertelem
  ($var ($name ($mrv4)) ($ty (!13))) // result
  ($var ($name ($__v:51)) ($ty (!13)))  // aggregate value
  ($var ($name ($mrv)) ($ty (!14))) // element
  ($var ($name ($__v:50)) ($ty (!20))) // offset
  ($debug ($srcloc ($line (#15)) ($file (!15)))))
```

##### AR_Extract_Element

```
($extractelem
  ($var ($name ($mrv_gr)) ($ty (!14))) // result
  ($var ($name ($tmp)) ($ty (!13))) // aggregate value
  ($var ($name ($__v:4)) ($ty (!20))) // offset
  ($debug ($srcloc ($line (#20)) ($file (!15)))))
```

##### AR_Return_Value

```
($ret ($rhs ($cst ($constantint ($val (#0)) ($ty (!2))))) ($debug ($srcloc ($line (#9)) ($file (!8)))))
```

##### AR_Call

```
($call
  ($ret ($var ($name ($main._4)) ($ty (!9))))
  ($callee ($var ($name ($__v:5)) ($ty (!12))))
  ($args ($a ($cst ($constantint ($val (#1)) ($ty (!9))))))
  ($inlineAsm ($true))
  ($debug ($srcloc ($line (#7)) ($file (!11)))))
```

##### AR_Invoke

```
($invoke
  ($call ($ret ($var ($name ($__v:119)) ($ty (!34))))
         ($callee ($cst ($constantfunctionptr ($name ($_Z3runP1A)) ($ty (!53)))))
         ($args ($a ($var ($name ($tmp)) ($ty (!35)))))
         ($debug ($srcloc ($line (#45)) ($file (!13)))))
  ($br ($normal ($invcont)) ($exception ($lpad)))
  ($debug ($srcloc ($line (#45)) ($file (!13)))))
```

##### AR_Abstract_Memory

```
($abstractmemory
  ($ptr ($cst ($globalvariableref ($name ($_ZTVN10__cxxabiv117__class_type_infoE)) ($gv (!4)) ($ty (!21)))))
  ($len ($cst ($constantint ($val (#0)) ($ty (!20)))))
  ($debug ($srcloc ($line (#-1)) ($file (!13)))))
```

##### AR_Abstract_Variable

```
($abstractvariable ($var ($name ($__v:63)) ($ty (!24))) ($debug ($srcloc ($line (#31)) ($file (!27)))))
```

##### AR_Landing_Pad

```
($def (!10 ($type ($ptr ($ty (!9)) ($realsize (#64)) ($storesize (#8))))))

($def (!18 ($type ($int ($realsize (#32)) ($storesize (#4))))))

($def (!42 ($type ($struct
                    ($layout (#0 (!10))
                             (#8 (!18)))
                    ($align (#64))
                    ($realsize (#128))
                    ($storesize (#16))))))

($landingpad
  ($exception
    ($var
      ($name ($main._invoke.cont.3)) ($ty (!42))))
  ($debug ($srcloc ($line (#27)) ($file (!34)))))
```

##### AR_Resume

```
($resume
       ($exception
          ($var ($name ($__v:173)) ($ty (!40))))
       ($debug ($srcloc ($line (#22)) ($file (!48)))))

```

##### AR_Unreachable

```
($unreachable ($debug ($srcloc ($line (#48)) ($file (!27)))))
```

#### AR_Function

```
($function ($name ()) ($ty ()) ($params () ...) ($local_vars () ...) ($code ($basicblock) ...))
```

#### AR_Code

```
($code
  ($entry ($entry))
  ($exit ($return))
  ($unreachable ($if.then))
  ($ehresume ($eh.resume))
  ($basicblocks ($basicblock) ...)
  ($trans ($edge () ()) ...))
```

We support three different types of terminating blocks in AR_Code: `exit`, `unreachable`, and `ehresume`. In an `AR_Code`, we allow at most one basic block per type. Using the LLVM frontend, we transform the LLVM IR using the `UnifyFunctionExitNodes` pass to obtain the `exit` and `unreachable` blocks, where each represents the unified block for all returning and unreachable blocks, respectively. `ehresume` is already a unified block that contains the `resume` instruction to resume propagation of an exception whose unwinding was interrupted with the LLVM `landingpad` instruction. Note that ($exit), ($unreachable), and ($ehresume) may have zero arguments, which means these blocks may be absent in the function in the LLVM IR. If the exit block is missing, then this means the function does not terminate with a return block that ends with a return instruction.

#### AR_Basic_Block

```
($basicblock ($name ()) ($instructions () ...) ... )
```

### Metainfo provided by the LLVM frontend

```
($metainfo ($targettriple ($x86_64-apple-macosx10.11.0)))
($metainfo ($endianess ($LittleEndian)))
($metainfo ($sizeofptr (#64)))
```

### File

```
(!6 ($file) ($/path/to/some/directory/unitests/5/loop.c))
```

### AR_Source_Location

```
($srcloc ($line (#4)) ($col (#3)) ($file (!9)))
```
