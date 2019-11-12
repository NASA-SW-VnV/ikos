; ModuleID = 'bitwise-cond-2.pp.bc'
source_filename = "bitwise-cond-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

@g = common global i32 0, align 4, !dbg !0
; CHECK: define si32* @g, align 4, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @g, 0, align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @foo(i32, i32) #0 !dbg !12 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i32 %1, metadata !17, metadata !DIExpression()), !dbg !16
  %3 = sub nsw i32 %0, %1, !dbg !18
  call void @llvm.dbg.value(metadata i32 %3, metadata !19, metadata !DIExpression()), !dbg !16
  %4 = load i32, i32* @g, align 4, !dbg !20
  %5 = icmp eq i32 %4, 0, !dbg !22
  br i1 %5, label %6, label %10, !dbg !23

6:                                                ; preds = %2
  %7 = icmp ne i32 %3, 0, !dbg !24
  br i1 %7, label %8, label %10, !dbg !25

8:                                                ; preds = %6
  %9 = add nsw i32 %0, %1, !dbg !26
  call void @llvm.dbg.value(metadata i32 %9, metadata !27, metadata !DIExpression()), !dbg !16
  br label %12, !dbg !28

10:                                               ; preds = %6, %2
  %11 = mul nsw i32 %0, %1, !dbg !29
  call void @llvm.dbg.value(metadata i32 %11, metadata !27, metadata !DIExpression()), !dbg !16
  br label %12

12:                                               ; preds = %10, %8
  %.0 = phi i32 [ %9, %8 ], [ %11, %10 ], !dbg !30
  call void @llvm.dbg.value(metadata i32 %.0, metadata !27, metadata !DIExpression()), !dbg !16
  %13 = mul nsw i32 %.0, 42, !dbg !31
  ret i32 %13, !dbg !32
}
; CHECK: define si32 @foo(si32 %1, si32 %2) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32 %3 = %1 ssub.nw %2
; CHECK:   si32 %4 = load @g, align 4
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4, #5} {
; CHECK:   %4 sieq 0
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#6} {
; CHECK:   %4 sine 0
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#7} {
; CHECK:   %3 sine 0
; CHECK:   si32 %5 = %1 sadd.nw %2
; CHECK:   si32 %.0 = %5
; CHECK: }
; CHECK: #5 predecessors={#2} successors={#6} {
; CHECK:   %3 sieq 0
; CHECK: }
; CHECK: #6 predecessors={#3, #5} successors={#7} {
; CHECK:   si32 %6 = %1 smul.nw %2
; CHECK:   si32 %.0 = %6
; CHECK: }
; CHECK: #7 !exit predecessors={#4, #6} {
; CHECK:   si32 %7 = %.0 smul.nw 42
; CHECK:   return %7
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!7, !8, !9, !10}
!llvm.ident = !{!11}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "bitwise-cond-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!4 = !{}
!5 = !{!0}
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{i32 2, !"Dwarf Version", i32 4}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!9 = !{i32 1, !"wchar_size", i32 4}
!10 = !{i32 7, !"PIC Level", i32 2}
!11 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!12 = distinct !DISubprogram(name: "foo", scope: !3, file: !3, line: 3, type: !13, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!13 = !DISubroutineType(types: !14)
!14 = !{!6, !6, !6}
!15 = !DILocalVariable(name: "x", arg: 1, scope: !12, file: !3, line: 3, type: !6)
!16 = !DILocation(line: 0, scope: !12)
!17 = !DILocalVariable(name: "y", arg: 2, scope: !12, file: !3, line: 3, type: !6)
!18 = !DILocation(line: 4, column: 13, scope: !12)
!19 = !DILocalVariable(name: "z", scope: !12, file: !3, line: 4, type: !6)
!20 = !DILocation(line: 6, column: 7, scope: !21)
!21 = distinct !DILexicalBlock(scope: !12, file: !3, line: 6, column: 7)
!22 = !DILocation(line: 6, column: 9, scope: !21)
!23 = !DILocation(line: 6, column: 14, scope: !21)
!24 = !DILocation(line: 6, column: 17, scope: !21)
!25 = !DILocation(line: 6, column: 7, scope: !12)
!26 = !DILocation(line: 7, column: 11, scope: !21)
!27 = !DILocalVariable(name: "a", scope: !12, file: !3, line: 5, type: !6)
!28 = !DILocation(line: 7, column: 5, scope: !21)
!29 = !DILocation(line: 9, column: 11, scope: !21)
!30 = !DILocation(line: 0, scope: !21)
!31 = !DILocation(line: 10, column: 12, scope: !12)
!32 = !DILocation(line: 10, column: 3, scope: !12)
