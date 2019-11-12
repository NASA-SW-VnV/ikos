; ModuleID = 'bitwise-cond-1.pp.bc'
source_filename = "bitwise-cond-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @foo(i32, i32, i32) #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !12, metadata !DIExpression()), !dbg !13
  call void @llvm.dbg.value(metadata i32 %1, metadata !14, metadata !DIExpression()), !dbg !13
  call void @llvm.dbg.value(metadata i32 %2, metadata !15, metadata !DIExpression()), !dbg !13
  %4 = sub nsw i32 %0, %1, !dbg !16
  call void @llvm.dbg.value(metadata i32 %4, metadata !17, metadata !DIExpression()), !dbg !13
  %5 = icmp eq i32 %2, 0, !dbg !18
  br i1 %5, label %6, label %10, !dbg !19

6:                                                ; preds = %3
  %7 = icmp ne i32 %4, 0, !dbg !20
  br i1 %7, label %8, label %10, !dbg !21

8:                                                ; preds = %6
  %9 = add nsw i32 %0, %1, !dbg !22
  br label %12, !dbg !21

10:                                               ; preds = %6, %3
  %11 = add nsw i32 %1, %2, !dbg !23
  br label %12, !dbg !21

12:                                               ; preds = %10, %8
  %13 = phi i32 [ %9, %8 ], [ %11, %10 ], !dbg !21
  call void @llvm.dbg.value(metadata i32 %13, metadata !24, metadata !DIExpression()), !dbg !13
  %14 = icmp sgt i32 %4, %13, !dbg !25
  br i1 %14, label %15, label %16, !dbg !26

15:                                               ; preds = %12
  br label %17, !dbg !26

16:                                               ; preds = %12
  br label %17, !dbg !26

17:                                               ; preds = %16, %15
  %18 = phi i32 [ %0, %15 ], [ %1, %16 ], !dbg !26
  ret i32 %18, !dbg !27
}
; CHECK: define si32 @foo(si32 %1, si32 %2, si32 %3) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32 %4 = %1 ssub.nw %2
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4, #5} {
; CHECK:   %3 sieq 0
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#6} {
; CHECK:   %3 sine 0
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#7} {
; CHECK:   %4 sine 0
; CHECK:   si32 %5 = %1 sadd.nw %2
; CHECK:   si32 %6 = %5
; CHECK: }
; CHECK: #5 predecessors={#2} successors={#6} {
; CHECK:   %4 sieq 0
; CHECK: }
; CHECK: #6 predecessors={#3, #5} successors={#7} {
; CHECK:   si32 %7 = %2 sadd.nw %3
; CHECK:   si32 %6 = %7
; CHECK: }
; CHECK: #7 predecessors={#4, #6} successors={#8, #9} {
; CHECK: }
; CHECK: #8 predecessors={#7} successors={#10} {
; CHECK:   %4 sigt %6
; CHECK:   si32 %8 = %1
; CHECK: }
; CHECK: #9 predecessors={#7} successors={#10} {
; CHECK:   %4 sile %6
; CHECK:   si32 %8 = %2
; CHECK: }
; CHECK: #10 !exit predecessors={#8, #9} {
; CHECK:   return %8
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "bitwise-cond-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11, !11, !11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "x", arg: 1, scope: !8, file: !1, line: 1, type: !11)
!13 = !DILocation(line: 0, scope: !8)
!14 = !DILocalVariable(name: "y", arg: 2, scope: !8, file: !1, line: 1, type: !11)
!15 = !DILocalVariable(name: "z", arg: 3, scope: !8, file: !1, line: 1, type: !11)
!16 = !DILocation(line: 2, column: 13, scope: !8)
!17 = !DILocalVariable(name: "a", scope: !8, file: !1, line: 2, type: !11)
!18 = !DILocation(line: 3, column: 14, scope: !8)
!19 = !DILocation(line: 3, column: 19, scope: !8)
!20 = !DILocation(line: 3, column: 22, scope: !8)
!21 = !DILocation(line: 3, column: 11, scope: !8)
!22 = !DILocation(line: 3, column: 29, scope: !8)
!23 = !DILocation(line: 3, column: 37, scope: !8)
!24 = !DILocalVariable(name: "b", scope: !8, file: !1, line: 3, type: !11)
!25 = !DILocation(line: 4, column: 13, scope: !8)
!26 = !DILocation(line: 4, column: 10, scope: !8)
!27 = !DILocation(line: 4, column: 3, scope: !8)
