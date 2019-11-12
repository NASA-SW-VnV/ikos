; ModuleID = 'empty-array-2.pp.bc'
source_filename = "empty-array-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.anon.0 = type { [0 x i32], %struct.anon }
%struct.anon = type { i32 }

@c = global %struct.anon.0 { [0 x i32] zeroinitializer, %struct.anon { i32 6 } }, align 4, !dbg !0
; CHECK: define {0: [0 x si32], 0: {0: si32}}* @c, align 4, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @c, {0: aggregate_zero, 0: {0: 6}}, align 1
; CHECK: }
; CHECK: }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!17, !18, !19, !20}
!llvm.ident = !{!21}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !3, line: 6, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "empty-array-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!4 = !{}
!5 = !{!0}
!6 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !3, line: 1, size: 32, elements: !7)
!7 = !{!8, !13}
!8 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !6, file: !3, line: 2, baseType: !9)
!9 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, elements: !11)
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !{!12}
!12 = !DISubrange(count: 0)
!13 = !DIDerivedType(tag: DW_TAG_member, scope: !6, file: !3, line: 3, baseType: !14, size: 32)
!14 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !6, file: !3, line: 3, size: 32, elements: !15)
!15 = !{!16}
!16 = !DIDerivedType(tag: DW_TAG_member, name: "b", scope: !14, file: !3, line: 4, baseType: !10, size: 32)
!17 = !{i32 2, !"Dwarf Version", i32 4}
!18 = !{i32 2, !"Debug Info Version", i32 3}
!19 = !{i32 1, !"wchar_size", i32 4}
!20 = !{i32 7, !"PIC Level", i32 2}
!21 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
