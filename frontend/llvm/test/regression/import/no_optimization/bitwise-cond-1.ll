; ModuleID = 'bitwise-cond-1.c.pp.bc'
source_filename = "bitwise-cond-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @foo(i32, i32, i32) #0 !dbg !7 {
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !11, metadata !12), !dbg !13
  store i32 %1, i32* %5, align 4
  call void @llvm.dbg.declare(metadata i32* %5, metadata !14, metadata !12), !dbg !15
  store i32 %2, i32* %6, align 4
  call void @llvm.dbg.declare(metadata i32* %6, metadata !16, metadata !12), !dbg !17
  call void @llvm.dbg.declare(metadata i32* %7, metadata !18, metadata !12), !dbg !19
  %9 = load i32, i32* %4, align 4, !dbg !20
  %10 = load i32, i32* %5, align 4, !dbg !21
  %11 = sub nsw i32 %9, %10, !dbg !22
  store i32 %11, i32* %7, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata i32* %8, metadata !23, metadata !12), !dbg !24
  %12 = load i32, i32* %6, align 4, !dbg !25
  %13 = icmp eq i32 %12, 0, !dbg !26
  br i1 %13, label %14, label %21, !dbg !27

; <label>:14:                                     ; preds = %3
  %15 = load i32, i32* %7, align 4, !dbg !28
  %16 = icmp ne i32 %15, 0, !dbg !28
  br i1 %16, label %17, label %21, !dbg !30

; <label>:17:                                     ; preds = %14
  %18 = load i32, i32* %4, align 4, !dbg !31
  %19 = load i32, i32* %5, align 4, !dbg !33
  %20 = add nsw i32 %18, %19, !dbg !34
  br label %25, !dbg !35

; <label>:21:                                     ; preds = %14, %3
  %22 = load i32, i32* %5, align 4, !dbg !36
  %23 = load i32, i32* %6, align 4, !dbg !38
  %24 = add nsw i32 %22, %23, !dbg !39
  br label %25, !dbg !40

; <label>:25:                                     ; preds = %21, %17
  %26 = phi i32 [ %20, %17 ], [ %24, %21 ], !dbg !41
  store i32 %26, i32* %8, align 4, !dbg !43
  %27 = load i32, i32* %7, align 4, !dbg !44
  %28 = load i32, i32* %8, align 4, !dbg !45
  %29 = icmp sgt i32 %27, %28, !dbg !46
  br i1 %29, label %30, label %32, !dbg !47

; <label>:30:                                     ; preds = %25
  %31 = load i32, i32* %4, align 4, !dbg !48
  br label %34, !dbg !49

; <label>:32:                                     ; preds = %25
  %33 = load i32, i32* %5, align 4, !dbg !50
  br label %34, !dbg !51

; <label>:34:                                     ; preds = %32, %30
  %35 = phi i32 [ %31, %30 ], [ %33, %32 ], !dbg !52
  ret i32 %35, !dbg !53
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "bitwise-cond-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !10, !10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "x", arg: 1, scope: !7, file: !1, line: 1, type: !10)
!12 = !DIExpression()
!13 = !DILocation(line: 1, column: 13, scope: !7)
!14 = !DILocalVariable(name: "y", arg: 2, scope: !7, file: !1, line: 1, type: !10)
!15 = !DILocation(line: 1, column: 20, scope: !7)
!16 = !DILocalVariable(name: "z", arg: 3, scope: !7, file: !1, line: 1, type: !10)
!17 = !DILocation(line: 1, column: 27, scope: !7)
!18 = !DILocalVariable(name: "a", scope: !7, file: !1, line: 2, type: !10)
!19 = !DILocation(line: 2, column: 7, scope: !7)
!20 = !DILocation(line: 2, column: 11, scope: !7)
!21 = !DILocation(line: 2, column: 15, scope: !7)
!22 = !DILocation(line: 2, column: 13, scope: !7)
!23 = !DILocalVariable(name: "b", scope: !7, file: !1, line: 3, type: !10)
!24 = !DILocation(line: 3, column: 7, scope: !7)
!25 = !DILocation(line: 3, column: 12, scope: !7)
!26 = !DILocation(line: 3, column: 14, scope: !7)
!27 = !DILocation(line: 3, column: 19, scope: !7)
!28 = !DILocation(line: 3, column: 22, scope: !29)
!29 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 1)
!30 = !DILocation(line: 3, column: 11, scope: !29)
!31 = !DILocation(line: 3, column: 27, scope: !32)
!32 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 2)
!33 = !DILocation(line: 3, column: 31, scope: !32)
!34 = !DILocation(line: 3, column: 29, scope: !32)
!35 = !DILocation(line: 3, column: 11, scope: !32)
!36 = !DILocation(line: 3, column: 35, scope: !37)
!37 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 3)
!38 = !DILocation(line: 3, column: 39, scope: !37)
!39 = !DILocation(line: 3, column: 37, scope: !37)
!40 = !DILocation(line: 3, column: 11, scope: !37)
!41 = !DILocation(line: 3, column: 11, scope: !42)
!42 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 4)
!43 = !DILocation(line: 3, column: 7, scope: !42)
!44 = !DILocation(line: 4, column: 11, scope: !7)
!45 = !DILocation(line: 4, column: 15, scope: !7)
!46 = !DILocation(line: 4, column: 13, scope: !7)
!47 = !DILocation(line: 4, column: 10, scope: !7)
!48 = !DILocation(line: 4, column: 20, scope: !29)
!49 = !DILocation(line: 4, column: 10, scope: !29)
!50 = !DILocation(line: 4, column: 24, scope: !32)
!51 = !DILocation(line: 4, column: 10, scope: !32)
!52 = !DILocation(line: 4, column: 10, scope: !37)
!53 = !DILocation(line: 4, column: 3, scope: !37)
