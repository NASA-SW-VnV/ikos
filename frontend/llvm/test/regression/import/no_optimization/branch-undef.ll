; ModuleID = 'branch-undef.pp.bc'
source_filename = "branch-undef.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !8 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  br i1 true, label %2, label %7, !dbg !12

2:                                                ; preds = %0
  br i1 undef, label %3, label %4, !dbg !13

3:                                                ; preds = %2
  br label %5, !dbg !13

4:                                                ; preds = %2
  br label %5, !dbg !13

5:                                                ; preds = %4, %3
  %6 = phi i32 [ undef, %3 ], [ 1, %4 ], !dbg !13
  br label %8, !dbg !12

7:                                                ; preds = %0
  br label %8, !dbg !12

8:                                                ; preds = %7, %5
  %9 = phi i32 [ %6, %5 ], [ 2, %7 ], !dbg !12
  ret i32 %9, !dbg !14
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   store $1, 0, align 4
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   si32 %2 = undef
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   si32 %2 = 1
; CHECK: }
; CHECK: #5 !exit predecessors={#4} {
; CHECK:   return %3
; CHECK: }
; CHECK: #4 predecessors={#2, #3} successors={#5} {
; CHECK:   si32 %3 = %2
; CHECK: }
; CHECK: }

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "branch-undef.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !9, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocation(line: 2, column: 10, scope: !8)
!13 = !DILocation(line: 2, column: 17, scope: !8)
!14 = !DILocation(line: 2, column: 3, scope: !8)
