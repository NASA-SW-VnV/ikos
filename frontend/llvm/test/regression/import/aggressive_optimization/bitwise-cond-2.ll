; ModuleID = 'bitwise-cond-2.c.pp.bc'
source_filename = "bitwise-cond-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!7, !8, !9}
!llvm.ident = !{!10}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3)
!1 = !DIFile(filename: "bitwise-cond-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4}
!4 = !DIGlobalVariableExpression(var: !5)
!5 = distinct !DIGlobalVariable(name: "g", scope: !0, file: !1, line: 1, type: !6, isLocal: false, isDefinition: true)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{i32 2, !"Dwarf Version", i32 4}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!9 = !{i32 1, !"PIC Level", i32 2}
!10 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
