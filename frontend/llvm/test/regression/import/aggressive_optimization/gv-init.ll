; ModuleID = 'gv-init.pp.bc'
source_filename = "gv-init.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !24 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !30, metadata !DIExpression()), !dbg !31
  call void @llvm.dbg.value(metadata i8** %1, metadata !32, metadata !DIExpression()), !dbg !33
  call void @llvm.dbg.value(metadata i32 0, metadata !34, metadata !DIExpression()), !dbg !35
  call void @llvm.dbg.value(metadata i32 0, metadata !36, metadata !DIExpression()), !dbg !37
  br label %3, !dbg !38

; <label>:3:                                      ; preds = %8, %2
  %.01 = phi i32 [ 0, %2 ], [ %9, %8 ], !dbg !39
  %.0 = phi i32 [ 0, %2 ], [ %.1.lcssa, %8 ], !dbg !37
  call void @llvm.dbg.value(metadata i32 %.0, metadata !36, metadata !DIExpression()), !dbg !37
  call void @llvm.dbg.value(metadata i32 %.01, metadata !34, metadata !DIExpression()), !dbg !35
  %4 = icmp ult i32 %.01, 100, !dbg !40
  br i1 %4, label %.preheader, label %10, !dbg !43

.preheader:                                       ; preds = %3, %6
  %.1 = phi i32 [ %7, %6 ], [ %.0, %3 ], !dbg !39
  call void @llvm.dbg.value(metadata i32 %.1, metadata !36, metadata !DIExpression()), !dbg !37
  %5 = icmp slt i32 %.1, 100, !dbg !44
  br i1 %5, label %6, label %8, !dbg !47

; <label>:6:                                      ; preds = %.preheader
  %7 = add nsw i32 %.1, 1, !dbg !48
  call void @llvm.dbg.value(metadata i32 %7, metadata !36, metadata !DIExpression()), !dbg !37
  br label %.preheader, !dbg !49, !llvm.loop !50

; <label>:8:                                      ; preds = %.preheader
  %.1.lcssa = phi i32 [ %.1, %.preheader ], !dbg !39
  call void @llvm.dbg.value(metadata i32 %.1.lcssa, metadata !36, metadata !DIExpression()), !dbg !37
  %9 = add nuw nsw i32 %.01, 1, !dbg !52
  call void @llvm.dbg.value(metadata i32 %9, metadata !34, metadata !DIExpression()), !dbg !35
  br label %3, !dbg !53, !llvm.loop !54

; <label>:10:                                     ; preds = %3
  ret i32 0, !dbg !56
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32 %.01 = 0
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #6} successors={#3, #4} {
; CHECK:   ui32 %3 = bitcast %.01
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#.preheader} {
; CHECK:   %3 uilt 100
; CHECK:   si32 %.1 = %.0
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %3 uige 100
; CHECK:   return 0
; CHECK: }
; CHECK: #.preheader predecessors={#3, #5} successors={#5, #6} {
; CHECK: }
; CHECK: #5 predecessors={#.preheader} successors={#.preheader} {
; CHECK:   %.1 silt 100
; CHECK:   si32 %4 = %.1 sadd.nw 1
; CHECK:   si32 %.1 = %4
; CHECK: }
; CHECK: #6 predecessors={#.preheader} successors={#2} {
; CHECK:   %.1 sige 100
; CHECK:   si32 %.1.lcssa = %.1
; CHECK:   si32 %5 = %.01 sadd.nw 1
; CHECK:   si32 %.01 = %5
; CHECK:   si32 %.0 = %.1.lcssa
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!19, !20, !21, !22}
!llvm.ident = !{!23}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 (tags/RELEASE_800/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "gv-init.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4, !10, !12, !17}
!4 = !DIGlobalVariableExpression(var: !5, expr: !DIExpression())
!5 = distinct !DIGlobalVariable(name: "b", scope: !0, file: !1, line: 2, type: !6, isLocal: false, isDefinition: true)
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 64, elements: !8)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DISubrange(count: 2)
!10 = !DIGlobalVariableExpression(var: !11, expr: !DIExpression())
!11 = distinct !DIGlobalVariable(name: "d", scope: !0, file: !1, line: 4, type: !7, isLocal: false, isDefinition: true)
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(name: "a", scope: !0, file: !1, line: 1, type: !14, isLocal: false, isDefinition: true)
!14 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 320000, elements: !15)
!15 = !{!16, !16}
!16 = !DISubrange(count: 100)
!17 = !DIGlobalVariableExpression(var: !18, expr: !DIExpression())
!18 = distinct !DIGlobalVariable(name: "c", scope: !0, file: !1, line: 3, type: !7, isLocal: false, isDefinition: true)
!19 = !{i32 2, !"Dwarf Version", i32 4}
!20 = !{i32 2, !"Debug Info Version", i32 3}
!21 = !{i32 1, !"wchar_size", i32 4}
!22 = !{i32 7, !"PIC Level", i32 2}
!23 = !{!"clang version 8.0.0 (tags/RELEASE_800/final)"}
!24 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 7, type: !25, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!25 = !DISubroutineType(types: !26)
!26 = !{!7, !7, !27}
!27 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !28, size: 64)
!28 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64)
!29 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!30 = !DILocalVariable(name: "argc", arg: 1, scope: !24, file: !1, line: 7, type: !7)
!31 = !DILocation(line: 7, column: 14, scope: !24)
!32 = !DILocalVariable(name: "argv", arg: 2, scope: !24, file: !1, line: 7, type: !27)
!33 = !DILocation(line: 7, column: 27, scope: !24)
!34 = !DILocalVariable(name: "i", scope: !24, file: !1, line: 8, type: !7)
!35 = !DILocation(line: 8, column: 7, scope: !24)
!36 = !DILocalVariable(name: "j", scope: !24, file: !1, line: 8, type: !7)
!37 = !DILocation(line: 8, column: 14, scope: !24)
!38 = !DILocation(line: 9, column: 3, scope: !24)
!39 = !DILocation(line: 0, scope: !24)
!40 = !DILocation(line: 9, column: 12, scope: !41)
!41 = distinct !DILexicalBlock(scope: !42, file: !1, line: 9, column: 3)
!42 = distinct !DILexicalBlock(scope: !24, file: !1, line: 9, column: 3)
!43 = !DILocation(line: 9, column: 3, scope: !42)
!44 = !DILocation(line: 10, column: 14, scope: !45)
!45 = distinct !DILexicalBlock(scope: !46, file: !1, line: 10, column: 5)
!46 = distinct !DILexicalBlock(scope: !41, file: !1, line: 10, column: 5)
!47 = !DILocation(line: 10, column: 5, scope: !46)
!48 = !DILocation(line: 10, column: 22, scope: !45)
!49 = !DILocation(line: 10, column: 5, scope: !45)
!50 = distinct !{!50, !47, !51}
!51 = !DILocation(line: 14, column: 30, scope: !46)
!52 = !DILocation(line: 9, column: 20, scope: !41)
!53 = !DILocation(line: 9, column: 3, scope: !41)
!54 = distinct !{!54, !43, !55}
!55 = !DILocation(line: 14, column: 30, scope: !42)
!56 = !DILocation(line: 15, column: 3, scope: !24)
