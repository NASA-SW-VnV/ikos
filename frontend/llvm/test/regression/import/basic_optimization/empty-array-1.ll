; ModuleID = 'empty-array-1.pp.bc'
source_filename = "empty-array-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.anon = type { [0 x i8], i32*, double }

@d = global %struct.anon { [0 x i8] zeroinitializer, i32* inttoptr (i64 1 to i32*), double 0.000000e+00 }, align 8, !dbg !0
; CHECK: define {0: [0 x si8], 0: si32*, 8: double}* @d, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si32* %1 = sitoptr 1
; CHECK:   store @d, {0: aggregate_zero, 0: %1, 8: 0.0E+0}, align 1
; CHECK: }
; CHECK: }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!18, !19, !20, !21}
!llvm.ident = !{!22}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "d", scope: !2, file: !3, line: 5, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "empty-array-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!4 = !{}
!5 = !{!0}
!6 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !3, line: 1, size: 128, elements: !7)
!7 = !{!8, !13, !16}
!8 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !6, file: !3, line: 2, baseType: !9)
!9 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, elements: !11)
!10 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!11 = !{!12}
!12 = !DISubrange(count: 0)
!13 = !DIDerivedType(tag: DW_TAG_member, name: "b", scope: !6, file: !3, line: 3, baseType: !14, size: 64)
!14 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !15, size: 64)
!15 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!16 = !DIDerivedType(tag: DW_TAG_member, name: "c", scope: !6, file: !3, line: 4, baseType: !17, size: 64, offset: 64)
!17 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!18 = !{i32 2, !"Dwarf Version", i32 4}
!19 = !{i32 2, !"Debug Info Version", i32 3}
!20 = !{i32 1, !"wchar_size", i32 4}
!21 = !{i32 7, !"PIC Level", i32 2}
!22 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
