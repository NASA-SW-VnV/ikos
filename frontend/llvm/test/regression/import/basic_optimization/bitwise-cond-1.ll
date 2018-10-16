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
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !11, metadata !12), !dbg !13
  call void @llvm.dbg.value(metadata i32 %1, i64 0, metadata !14, metadata !12), !dbg !15
  call void @llvm.dbg.value(metadata i32 %2, i64 0, metadata !16, metadata !12), !dbg !17
  %4 = sub nsw i32 %0, %1, !dbg !18
  call void @llvm.dbg.value(metadata i32 %4, i64 0, metadata !19, metadata !12), !dbg !20
  %5 = icmp eq i32 %2, 0, !dbg !21
  br i1 %5, label %6, label %10, !dbg !22

; <label>:6:                                      ; preds = %3
  %7 = icmp ne i32 %4, 0, !dbg !23
  br i1 %7, label %8, label %10, !dbg !25

; <label>:8:                                      ; preds = %6
  %9 = add nsw i32 %0, %1, !dbg !26
  br label %12, !dbg !28

; <label>:10:                                     ; preds = %6, %3
  %11 = add nsw i32 %1, %2, !dbg !29
  br label %12, !dbg !31

; <label>:12:                                     ; preds = %10, %8
  %13 = phi i32 [ %9, %8 ], [ %11, %10 ], !dbg !32
  call void @llvm.dbg.value(metadata i32 %13, i64 0, metadata !34, metadata !12), !dbg !35
  %14 = icmp sgt i32 %4, %13, !dbg !36
  br i1 %14, label %15, label %16, !dbg !37

; <label>:15:                                     ; preds = %12
  br label %17, !dbg !38

; <label>:16:                                     ; preds = %12
  br label %17, !dbg !39

; <label>:17:                                     ; preds = %16, %15
  %18 = phi i32 [ %0, %15 ], [ %1, %16 ], !dbg !40
  ret i32 %18, !dbg !41
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "bitwise-cond-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
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
!18 = !DILocation(line: 2, column: 13, scope: !7)
!19 = !DILocalVariable(name: "a", scope: !7, file: !1, line: 2, type: !10)
!20 = !DILocation(line: 2, column: 7, scope: !7)
!21 = !DILocation(line: 3, column: 14, scope: !7)
!22 = !DILocation(line: 3, column: 19, scope: !7)
!23 = !DILocation(line: 3, column: 22, scope: !24)
!24 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 1)
!25 = !DILocation(line: 3, column: 11, scope: !24)
!26 = !DILocation(line: 3, column: 29, scope: !27)
!27 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 2)
!28 = !DILocation(line: 3, column: 11, scope: !27)
!29 = !DILocation(line: 3, column: 37, scope: !30)
!30 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 3)
!31 = !DILocation(line: 3, column: 11, scope: !30)
!32 = !DILocation(line: 3, column: 11, scope: !33)
!33 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 4)
!34 = !DILocalVariable(name: "b", scope: !7, file: !1, line: 3, type: !10)
!35 = !DILocation(line: 3, column: 7, scope: !7)
!36 = !DILocation(line: 4, column: 13, scope: !7)
!37 = !DILocation(line: 4, column: 10, scope: !7)
!38 = !DILocation(line: 4, column: 10, scope: !24)
!39 = !DILocation(line: 4, column: 10, scope: !27)
!40 = !DILocation(line: 4, column: 10, scope: !30)
!41 = !DILocation(line: 4, column: 3, scope: !30)
