; ModuleID = 'non-term-2.c.pp.bc'
source_filename = "non-term-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

@.str = private unnamed_addr constant [13 x i8] c"hello world\0A\00", align 1
; CHECK: define [13 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [104, 101, 108, 108, 111, 32, 119, 111, 114, 108, 100, 10, 0], align 1
; CHECK: }
; CHECK: }

declare i32 @printf(i8*, ...) #1
; CHECK: declare si32 @ar.libc.printf(si8*, ...)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !7 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  br label %2, !dbg !11

; <label>:2:                                      ; preds = %0, %2
  %3 = getelementptr inbounds [13 x i8], [13 x i8]* @.str, i32 0, i32 0, !dbg !12
  %4 = call i32 (i8*, ...) @printf(i8* %3), !dbg !12
  br label %2, !dbg !14, !llvm.loop !16
                                                  ; No predecessors!
  %6 = load i32, i32* %1, align 4, !dbg !18
  ret i32 %6, !dbg !18
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   store $1, 0, align 4
; CHECK: }
; CHECK: #2 predecessors={#1, #2} successors={#2} {
; CHECK:   si8* %2 = ptrshift @.str, 13 * 0, 1 * 0
; CHECK:   si32 %3 = call @ar.libc.printf(%2)
; CHECK: }
; CHECK: }

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "non-term-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !8, isLocal: false, isDefinition: true, scopeLine: 3, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocation(line: 4, column: 3, scope: !7)
!12 = !DILocation(line: 5, column: 5, scope: !13)
!13 = distinct !DILexicalBlock(scope: !7, file: !1, line: 4, column: 13)
!14 = !DILocation(line: 4, column: 3, scope: !15)
!15 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 1)
!16 = distinct !{!16, !11, !17}
!17 = !DILocation(line: 6, column: 3, scope: !7)
!18 = !DILocation(line: 8, column: 1, scope: !7)
