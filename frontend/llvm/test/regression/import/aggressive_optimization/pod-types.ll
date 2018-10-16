; ModuleID = 'pod-types.c.pp.bc'
source_filename = "pod-types.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !34 {
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !40, metadata !41), !dbg !42
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !43, metadata !41), !dbg !44
  call void @llvm.dbg.declare(metadata [10 x i32]* undef, metadata !45, metadata !41), !dbg !48
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !45, metadata !49), !dbg !48
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !45, metadata !50), !dbg !48
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !45, metadata !51), !dbg !48
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !45, metadata !52), !dbg !48
  call void @llvm.dbg.value(metadata i32 1, i64 0, metadata !45, metadata !49), !dbg !48
  call void @llvm.dbg.value(metadata i32 -1, i64 0, metadata !45, metadata !50), !dbg !48
  call void @llvm.dbg.value(metadata i32 255, i64 0, metadata !45, metadata !51), !dbg !48
  call void @llvm.dbg.value(metadata i32 42, i64 0, metadata !45, metadata !52), !dbg !48
  ret i32 0, !dbg !53
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!30, !31, !32}
!llvm.ident = !{!33}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3)
!1 = !DIFile(filename: "pod-types.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4, !7, !10, !13, !16, !20, !23}
!4 = !DIGlobalVariableExpression(var: !5)
!5 = distinct !DIGlobalVariable(name: "i", scope: !0, file: !1, line: 1, type: !6, isLocal: false, isDefinition: true)
!6 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!7 = !DIGlobalVariableExpression(var: !8)
!8 = distinct !DIGlobalVariable(name: "f", scope: !0, file: !1, line: 3, type: !9, isLocal: false, isDefinition: true)
!9 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!10 = !DIGlobalVariableExpression(var: !11)
!11 = distinct !DIGlobalVariable(name: "d", scope: !0, file: !1, line: 5, type: !12, isLocal: false, isDefinition: true)
!12 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!13 = !DIGlobalVariableExpression(var: !14)
!14 = distinct !DIGlobalVariable(name: "p", scope: !0, file: !1, line: 7, type: !15, isLocal: false, isDefinition: true)
!15 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!16 = !DIGlobalVariableExpression(var: !17)
!17 = distinct !DIGlobalVariable(name: "q", scope: !0, file: !1, line: 9, type: !18, isLocal: false, isDefinition: true)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!19 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!20 = !DIGlobalVariableExpression(var: !21)
!21 = distinct !DIGlobalVariable(name: "b", scope: !0, file: !1, line: 11, type: !22, isLocal: false, isDefinition: true)
!22 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!23 = !DIGlobalVariableExpression(var: !24)
!24 = distinct !DIGlobalVariable(name: "tab", scope: !0, file: !1, line: 13, type: !25, isLocal: false, isDefinition: true)
!25 = !DICompositeType(tag: DW_TAG_array_type, baseType: !26, size: 1920, elements: !27)
!26 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!27 = !{!28, !29}
!28 = !DISubrange(count: 10)
!29 = !DISubrange(count: 12)
!30 = !{i32 2, !"Dwarf Version", i32 4}
!31 = !{i32 2, !"Debug Info Version", i32 3}
!32 = !{i32 1, !"PIC Level", i32 2}
!33 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!34 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 17, type: !35, isLocal: false, isDefinition: true, scopeLine: 17, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!35 = !DISubroutineType(types: !36)
!36 = !{!19, !19, !37}
!37 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !38, size: 64)
!38 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !39, size: 64)
!39 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!40 = !DILocalVariable(name: "argc", arg: 1, scope: !34, file: !1, line: 17, type: !19)
!41 = !DIExpression()
!42 = !DILocation(line: 17, column: 14, scope: !34)
!43 = !DILocalVariable(name: "argv", arg: 2, scope: !34, file: !1, line: 17, type: !37)
!44 = !DILocation(line: 17, column: 27, scope: !34)
!45 = !DILocalVariable(name: "xxx", scope: !34, file: !1, line: 18, type: !46)
!46 = !DICompositeType(tag: DW_TAG_array_type, baseType: !19, size: 320, elements: !47)
!47 = !{!28}
!48 = !DILocation(line: 18, column: 7, scope: !34)
!49 = !DIExpression(DW_OP_LLVM_fragment, 0, 32)
!50 = !DIExpression(DW_OP_LLVM_fragment, 32, 32)
!51 = !DIExpression(DW_OP_LLVM_fragment, 64, 32)
!52 = !DIExpression(DW_OP_LLVM_fragment, 96, 32)
!53 = !DILocation(line: 19, column: 3, scope: !34)
