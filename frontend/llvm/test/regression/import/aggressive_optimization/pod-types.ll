; ModuleID = 'pod-types.pp.bc'
source_filename = "pod-types.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !35 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !41, metadata !DIExpression()), !dbg !42
  call void @llvm.dbg.value(metadata i8** %1, metadata !43, metadata !DIExpression()), !dbg !42
  call void @llvm.dbg.value(metadata i32 0, metadata !44, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 32)), !dbg !42
  call void @llvm.dbg.value(metadata i32 0, metadata !44, metadata !DIExpression(DW_OP_LLVM_fragment, 32, 32)), !dbg !42
  call void @llvm.dbg.value(metadata i32 0, metadata !44, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 32)), !dbg !42
  call void @llvm.dbg.value(metadata i32 0, metadata !44, metadata !DIExpression(DW_OP_LLVM_fragment, 96, 32)), !dbg !42
  call void @llvm.dbg.value(metadata i32 1, metadata !44, metadata !DIExpression(DW_OP_LLVM_fragment, 0, 32)), !dbg !42
  call void @llvm.dbg.value(metadata i32 -1, metadata !44, metadata !DIExpression(DW_OP_LLVM_fragment, 32, 32)), !dbg !42
  call void @llvm.dbg.value(metadata i32 255, metadata !44, metadata !DIExpression(DW_OP_LLVM_fragment, 64, 32)), !dbg !42
  call void @llvm.dbg.value(metadata i32 42, metadata !44, metadata !DIExpression(DW_OP_LLVM_fragment, 96, 32)), !dbg !42
  ret i32 0, !dbg !47
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!30, !31, !32, !33}
!llvm.ident = !{!34}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "pod-types.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4, !7, !10, !13, !16, !20, !23}
!4 = !DIGlobalVariableExpression(var: !5, expr: !DIExpression())
!5 = distinct !DIGlobalVariable(name: "i", scope: !0, file: !1, line: 1, type: !6, isLocal: false, isDefinition: true)
!6 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!7 = !DIGlobalVariableExpression(var: !8, expr: !DIExpression())
!8 = distinct !DIGlobalVariable(name: "f", scope: !0, file: !1, line: 3, type: !9, isLocal: false, isDefinition: true)
!9 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!10 = !DIGlobalVariableExpression(var: !11, expr: !DIExpression())
!11 = distinct !DIGlobalVariable(name: "d", scope: !0, file: !1, line: 5, type: !12, isLocal: false, isDefinition: true)
!12 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!13 = !DIGlobalVariableExpression(var: !14, expr: !DIExpression())
!14 = distinct !DIGlobalVariable(name: "p", scope: !0, file: !1, line: 7, type: !15, isLocal: false, isDefinition: true)
!15 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!16 = !DIGlobalVariableExpression(var: !17, expr: !DIExpression())
!17 = distinct !DIGlobalVariable(name: "q", scope: !0, file: !1, line: 9, type: !18, isLocal: false, isDefinition: true)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!19 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!20 = !DIGlobalVariableExpression(var: !21, expr: !DIExpression())
!21 = distinct !DIGlobalVariable(name: "b", scope: !0, file: !1, line: 11, type: !22, isLocal: false, isDefinition: true)
!22 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!23 = !DIGlobalVariableExpression(var: !24, expr: !DIExpression())
!24 = distinct !DIGlobalVariable(name: "tab", scope: !0, file: !1, line: 13, type: !25, isLocal: false, isDefinition: true)
!25 = !DICompositeType(tag: DW_TAG_array_type, baseType: !26, size: 1920, elements: !27)
!26 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!27 = !{!28, !29}
!28 = !DISubrange(count: 10)
!29 = !DISubrange(count: 12)
!30 = !{i32 2, !"Dwarf Version", i32 4}
!31 = !{i32 2, !"Debug Info Version", i32 3}
!32 = !{i32 1, !"wchar_size", i32 4}
!33 = !{i32 7, !"PIC Level", i32 2}
!34 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!35 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 17, type: !36, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!36 = !DISubroutineType(types: !37)
!37 = !{!19, !19, !38}
!38 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !39, size: 64)
!39 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !40, size: 64)
!40 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!41 = !DILocalVariable(name: "argc", arg: 1, scope: !35, file: !1, line: 17, type: !19)
!42 = !DILocation(line: 0, scope: !35)
!43 = !DILocalVariable(name: "argv", arg: 2, scope: !35, file: !1, line: 17, type: !38)
!44 = !DILocalVariable(name: "xxx", scope: !35, file: !1, line: 18, type: !45)
!45 = !DICompositeType(tag: DW_TAG_array_type, baseType: !19, size: 320, elements: !46)
!46 = !{!28}
!47 = !DILocation(line: 19, column: 3, scope: !35)
