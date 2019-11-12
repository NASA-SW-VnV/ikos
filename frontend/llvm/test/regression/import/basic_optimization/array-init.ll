; ModuleID = 'array-init.pp.bc'
source_filename = "array-init.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.i = type { [16 x i32] }

@k = global <{ %struct.i, { <{ i32, [15 x i32] }> } }> <{ %struct.i zeroinitializer, { <{ i32, [15 x i32] }> } { <{ i32, [15 x i32] }> <{ i32 2, [15 x i32] zeroinitializer }> } }>, align 16, !dbg !0
; CHECK: define <{0: {0: [16 x si32]}, 64: {0: <{0: si32, 4: [15 x si32]}>}}>* @k, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @k, {0: aggregate_zero, 64: {0: {0: 2, 4: aggregate_zero}}}, align 1
; CHECK: }
; CHECK: }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!16, !17, !18, !19}
!llvm.ident = !{!20}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "k", scope: !2, file: !3, line: 3, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "array-init.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!4 = !{}
!5 = !{!0}
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 1024, elements: !14)
!7 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "i", file: !3, line: 1, size: 512, flags: DIFlagTypePassByValue, elements: !8, identifier: "_ZTS1i")
!8 = !{!9}
!9 = !DIDerivedType(tag: DW_TAG_member, name: "j", scope: !7, file: !3, line: 2, baseType: !10, size: 512)
!10 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 512, elements: !12)
!11 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!12 = !{!13}
!13 = !DISubrange(count: 16)
!14 = !{!15}
!15 = !DISubrange(count: 2)
!16 = !{i32 2, !"Dwarf Version", i32 4}
!17 = !{i32 2, !"Debug Info Version", i32 3}
!18 = !{i32 1, !"wchar_size", i32 4}
!19 = !{i32 7, !"PIC Level", i32 2}
!20 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
