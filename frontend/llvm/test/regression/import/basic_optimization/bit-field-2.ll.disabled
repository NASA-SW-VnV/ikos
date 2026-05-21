; ModuleID = 'bit-field-2.pp.bc'
source_filename = "bit-field-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.anon = type { i8, i32 }

@b = global <{ [1 x { [4 x i8], i32 }], [1 x %struct.anon], [1 x %struct.anon], [1 x %struct.anon], [1 x %struct.anon], [1 x %struct.anon] }> <{ [1 x { [4 x i8], i32 }] [{ [4 x i8], i32 } { [4 x i8] undef, i32 0 }], [1 x %struct.anon] zeroinitializer, [1 x %struct.anon] zeroinitializer, [1 x %struct.anon] zeroinitializer, [1 x %struct.anon] zeroinitializer, [1 x %struct.anon] zeroinitializer }>, align 16, !dbg !0
; CHECK: define <{0: [1 x {0: [4 x si8], 4: si32}], 8: [1 x {0: si8, 4: si32}], 16: [1 x {0: si8, 4: si32}], 24: [1 x {0: si8, 4: si32}], 32: [1 x {0: si8, 4: si32}], 40: [1 x {0: si8, 4: si32}]}>* @b, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @b, {0: [{0: undef, 4: 0}], 8: aggregate_zero, 16: aggregate_zero, 24: aggregate_zero, 32: aggregate_zero, 40: aggregate_zero}, align 1
; CHECK: }
; CHECK: }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!14, !15, !16, !17}
!llvm.ident = !{!18}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 4, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "bit-field-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!4 = !{}
!5 = !{!0}
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 384, elements: !11)
!7 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !3, line: 1, size: 64, elements: !8)
!8 = !{!9}
!9 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !7, file: !3, line: 3, baseType: !10, size: 32, offset: 32)
!10 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!11 = !{!12, !13}
!12 = !DISubrange(count: 6)
!13 = !DISubrange(count: 1)
!14 = !{i32 2, !"Dwarf Version", i32 4}
!15 = !{i32 2, !"Debug Info Version", i32 3}
!16 = !{i32 1, !"wchar_size", i32 4}
!17 = !{i32 7, !"PIC Level", i32 2}
!18 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
