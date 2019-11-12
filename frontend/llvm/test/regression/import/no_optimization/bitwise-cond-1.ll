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
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !12, metadata !DIExpression()), !dbg !13
  store i32 %1, i32* %5, align 4
  call void @llvm.dbg.declare(metadata i32* %5, metadata !14, metadata !DIExpression()), !dbg !15
  store i32 %2, i32* %6, align 4
  call void @llvm.dbg.declare(metadata i32* %6, metadata !16, metadata !DIExpression()), !dbg !17
  call void @llvm.dbg.declare(metadata i32* %7, metadata !18, metadata !DIExpression()), !dbg !19
  %9 = load i32, i32* %4, align 4, !dbg !20
  %10 = load i32, i32* %5, align 4, !dbg !21
  %11 = sub nsw i32 %9, %10, !dbg !22
  store i32 %11, i32* %7, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata i32* %8, metadata !23, metadata !DIExpression()), !dbg !24
  %12 = load i32, i32* %6, align 4, !dbg !25
  %13 = icmp eq i32 %12, 0, !dbg !26
  br i1 %13, label %14, label %21, !dbg !27

14:                                               ; preds = %3
  %15 = load i32, i32* %7, align 4, !dbg !28
  %16 = icmp ne i32 %15, 0, !dbg !28
  br i1 %16, label %17, label %21, !dbg !29

17:                                               ; preds = %14
  %18 = load i32, i32* %4, align 4, !dbg !30
  %19 = load i32, i32* %5, align 4, !dbg !31
  %20 = add nsw i32 %18, %19, !dbg !32
  br label %25, !dbg !29

21:                                               ; preds = %14, %3
  %22 = load i32, i32* %5, align 4, !dbg !33
  %23 = load i32, i32* %6, align 4, !dbg !34
  %24 = add nsw i32 %22, %23, !dbg !35
  br label %25, !dbg !29

25:                                               ; preds = %21, %17
  %26 = phi i32 [ %20, %17 ], [ %24, %21 ], !dbg !29
  store i32 %26, i32* %8, align 4, !dbg !24
  %27 = load i32, i32* %7, align 4, !dbg !36
  %28 = load i32, i32* %8, align 4, !dbg !37
  %29 = icmp sgt i32 %27, %28, !dbg !38
  br i1 %29, label %30, label %32, !dbg !39

30:                                               ; preds = %25
  %31 = load i32, i32* %4, align 4, !dbg !40
  br label %34, !dbg !39

32:                                               ; preds = %25
  %33 = load i32, i32* %5, align 4, !dbg !41
  br label %34, !dbg !39

34:                                               ; preds = %32, %30
  %35 = phi i32 [ %31, %30 ], [ %33, %32 ], !dbg !39
  ret i32 %35, !dbg !42
}
; CHECK: define si32 @foo(si32 %1, si32 %2, si32 %3) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si32* $5 = allocate si32, 1, align 4
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   si32* $7 = allocate si32, 1, align 4
; CHECK:   si32* $8 = allocate si32, 1, align 4
; CHECK:   store $4, %1, align 4
; CHECK:   store $5, %2, align 4
; CHECK:   store $6, %3, align 4
; CHECK:   si32 %9 = load $4, align 4
; CHECK:   si32 %10 = load $5, align 4
; CHECK:   si32 %11 = %9 ssub.nw %10
; CHECK:   store $7, %11, align 4
; CHECK:   si32 %12 = load $6, align 4
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4, #5} {
; CHECK:   %12 sieq 0
; CHECK:   si32 %13 = load $7, align 4
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#6} {
; CHECK:   %12 sine 0
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#7} {
; CHECK:   %13 sine 0
; CHECK:   si32 %14 = load $4, align 4
; CHECK:   si32 %15 = load $5, align 4
; CHECK:   si32 %16 = %14 sadd.nw %15
; CHECK:   si32 %17 = %16
; CHECK: }
; CHECK: #5 predecessors={#2} successors={#6} {
; CHECK:   %13 sieq 0
; CHECK: }
; CHECK: #6 predecessors={#3, #5} successors={#7} {
; CHECK:   si32 %18 = load $5, align 4
; CHECK:   si32 %19 = load $6, align 4
; CHECK:   si32 %20 = %18 sadd.nw %19
; CHECK:   si32 %17 = %20
; CHECK: }
; CHECK: #7 predecessors={#4, #6} successors={#8, #9} {
; CHECK:   store $8, %17, align 4
; CHECK:   si32 %21 = load $7, align 4
; CHECK:   si32 %22 = load $8, align 4
; CHECK: }
; CHECK: #8 predecessors={#7} successors={#10} {
; CHECK:   %21 sigt %22
; CHECK:   si32 %23 = load $4, align 4
; CHECK:   si32 %24 = %23
; CHECK: }
; CHECK: #9 predecessors={#7} successors={#10} {
; CHECK:   %21 sile %22
; CHECK:   si32 %25 = load $5, align 4
; CHECK:   si32 %24 = %25
; CHECK: }
; CHECK: #10 !exit predecessors={#8, #9} {
; CHECK:   return %24
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "bitwise-cond-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!13 = !DILocation(line: 1, column: 13, scope: !8)
!14 = !DILocalVariable(name: "y", arg: 2, scope: !8, file: !1, line: 1, type: !11)
!15 = !DILocation(line: 1, column: 20, scope: !8)
!16 = !DILocalVariable(name: "z", arg: 3, scope: !8, file: !1, line: 1, type: !11)
!17 = !DILocation(line: 1, column: 27, scope: !8)
!18 = !DILocalVariable(name: "a", scope: !8, file: !1, line: 2, type: !11)
!19 = !DILocation(line: 2, column: 7, scope: !8)
!20 = !DILocation(line: 2, column: 11, scope: !8)
!21 = !DILocation(line: 2, column: 15, scope: !8)
!22 = !DILocation(line: 2, column: 13, scope: !8)
!23 = !DILocalVariable(name: "b", scope: !8, file: !1, line: 3, type: !11)
!24 = !DILocation(line: 3, column: 7, scope: !8)
!25 = !DILocation(line: 3, column: 12, scope: !8)
!26 = !DILocation(line: 3, column: 14, scope: !8)
!27 = !DILocation(line: 3, column: 19, scope: !8)
!28 = !DILocation(line: 3, column: 22, scope: !8)
!29 = !DILocation(line: 3, column: 11, scope: !8)
!30 = !DILocation(line: 3, column: 27, scope: !8)
!31 = !DILocation(line: 3, column: 31, scope: !8)
!32 = !DILocation(line: 3, column: 29, scope: !8)
!33 = !DILocation(line: 3, column: 35, scope: !8)
!34 = !DILocation(line: 3, column: 39, scope: !8)
!35 = !DILocation(line: 3, column: 37, scope: !8)
!36 = !DILocation(line: 4, column: 11, scope: !8)
!37 = !DILocation(line: 4, column: 15, scope: !8)
!38 = !DILocation(line: 4, column: 13, scope: !8)
!39 = !DILocation(line: 4, column: 10, scope: !8)
!40 = !DILocation(line: 4, column: 20, scope: !8)
!41 = !DILocation(line: 4, column: 24, scope: !8)
!42 = !DILocation(line: 4, column: 3, scope: !8)
