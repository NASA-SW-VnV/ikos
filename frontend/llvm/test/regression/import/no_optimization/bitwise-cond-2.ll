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
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  call void @llvm.dbg.declare(metadata i32* %3, metadata !15, metadata !DIExpression()), !dbg !16
  store i32 %1, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.declare(metadata i32* %5, metadata !19, metadata !DIExpression()), !dbg !20
  %7 = load i32, i32* %3, align 4, !dbg !21
  %8 = load i32, i32* %4, align 4, !dbg !22
  %9 = sub nsw i32 %7, %8, !dbg !23
  store i32 %9, i32* %5, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata i32* %6, metadata !24, metadata !DIExpression()), !dbg !25
  %10 = load i32, i32* @g, align 4, !dbg !26
  %11 = icmp eq i32 %10, 0, !dbg !28
  br i1 %11, label %12, label %19, !dbg !29

12:                                               ; preds = %2
  %13 = load i32, i32* %5, align 4, !dbg !30
  %14 = icmp ne i32 %13, 0, !dbg !30
  br i1 %14, label %15, label %19, !dbg !31

15:                                               ; preds = %12
  %16 = load i32, i32* %3, align 4, !dbg !32
  %17 = load i32, i32* %4, align 4, !dbg !33
  %18 = add nsw i32 %16, %17, !dbg !34
  store i32 %18, i32* %6, align 4, !dbg !35
  br label %23, !dbg !36

19:                                               ; preds = %12, %2
  %20 = load i32, i32* %3, align 4, !dbg !37
  %21 = load i32, i32* %4, align 4, !dbg !38
  %22 = mul nsw i32 %20, %21, !dbg !39
  store i32 %22, i32* %6, align 4, !dbg !40
  br label %23

23:                                               ; preds = %19, %15
  %24 = load i32, i32* %6, align 4, !dbg !41
  %25 = mul nsw i32 %24, 42, !dbg !42
  ret i32 %25, !dbg !43
}
; CHECK: define si32 @foo(si32 %1, si32 %2) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si32* $5 = allocate si32, 1, align 4
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   store $3, %1, align 4
; CHECK:   store $4, %2, align 4
; CHECK:   si32 %7 = load $3, align 4
; CHECK:   si32 %8 = load $4, align 4
; CHECK:   si32 %9 = %7 ssub.nw %8
; CHECK:   store $5, %9, align 4
; CHECK:   si32 %10 = load @g, align 4
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4, #5} {
; CHECK:   %10 sieq 0
; CHECK:   si32 %11 = load $5, align 4
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#6} {
; CHECK:   %10 sine 0
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#7} {
; CHECK:   %11 sine 0
; CHECK:   si32 %12 = load $3, align 4
; CHECK:   si32 %13 = load $4, align 4
; CHECK:   si32 %14 = %12 sadd.nw %13
; CHECK:   store $6, %14, align 4
; CHECK: }
; CHECK: #5 predecessors={#2} successors={#6} {
; CHECK:   %11 sieq 0
; CHECK: }
; CHECK: #6 predecessors={#3, #5} successors={#7} {
; CHECK:   si32 %15 = load $3, align 4
; CHECK:   si32 %16 = load $4, align 4
; CHECK:   si32 %17 = %15 smul.nw %16
; CHECK:   store $6, %17, align 4
; CHECK: }
; CHECK: #7 !exit predecessors={#6, #4} {
; CHECK:   si32 %18 = load $6, align 4
; CHECK:   si32 %19 = %18 smul.nw 42
; CHECK:   return %19
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!7, !8, !9, !10}
!llvm.ident = !{!11}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "g", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "bitwise-cond-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!16 = !DILocation(line: 3, column: 13, scope: !12)
!17 = !DILocalVariable(name: "y", arg: 2, scope: !12, file: !3, line: 3, type: !6)
!18 = !DILocation(line: 3, column: 20, scope: !12)
!19 = !DILocalVariable(name: "z", scope: !12, file: !3, line: 4, type: !6)
!20 = !DILocation(line: 4, column: 7, scope: !12)
!21 = !DILocation(line: 4, column: 11, scope: !12)
!22 = !DILocation(line: 4, column: 15, scope: !12)
!23 = !DILocation(line: 4, column: 13, scope: !12)
!24 = !DILocalVariable(name: "a", scope: !12, file: !3, line: 5, type: !6)
!25 = !DILocation(line: 5, column: 7, scope: !12)
!26 = !DILocation(line: 6, column: 7, scope: !27)
!27 = distinct !DILexicalBlock(scope: !12, file: !3, line: 6, column: 7)
!28 = !DILocation(line: 6, column: 9, scope: !27)
!29 = !DILocation(line: 6, column: 14, scope: !27)
!30 = !DILocation(line: 6, column: 17, scope: !27)
!31 = !DILocation(line: 6, column: 7, scope: !12)
!32 = !DILocation(line: 7, column: 9, scope: !27)
!33 = !DILocation(line: 7, column: 13, scope: !27)
!34 = !DILocation(line: 7, column: 11, scope: !27)
!35 = !DILocation(line: 7, column: 7, scope: !27)
!36 = !DILocation(line: 7, column: 5, scope: !27)
!37 = !DILocation(line: 9, column: 9, scope: !27)
!38 = !DILocation(line: 9, column: 13, scope: !27)
!39 = !DILocation(line: 9, column: 11, scope: !27)
!40 = !DILocation(line: 9, column: 7, scope: !27)
!41 = !DILocation(line: 10, column: 10, scope: !12)
!42 = !DILocation(line: 10, column: 12, scope: !12)
!43 = !DILocation(line: 10, column: 3, scope: !12)
