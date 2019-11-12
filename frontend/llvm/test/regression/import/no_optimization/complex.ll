; ModuleID = 'complex.pp.bc'
source_filename = "complex.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !8 {
  %1 = alloca i32, align 4
  %2 = alloca { double, double }, align 8
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata { double, double }* %2, metadata !12, metadata !DIExpression()), !dbg !14
  %3 = getelementptr inbounds { double, double }, { double, double }* %2, i32 0, i32 0, !dbg !14
  %4 = getelementptr inbounds { double, double }, { double, double }* %2, i32 0, i32 1, !dbg !14
  store double 1.000000e+00, double* %3, align 8, !dbg !14
  store double 2.000000e+00, double* %4, align 8, !dbg !14
  %5 = getelementptr inbounds { double, double }, { double, double }* %2, i32 0, i32 0, !dbg !15
  %6 = load double, double* %5, align 8, !dbg !15
  %7 = getelementptr inbounds { double, double }, { double, double }* %2, i32 0, i32 1, !dbg !15
  %8 = load double, double* %7, align 8, !dbg !15
  %9 = fptosi double %6 to i32, !dbg !16
  ret i32 %9, !dbg !17
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   {0: double, 8: double}* $2 = allocate {0: double, 8: double}, 1, align 8
; CHECK:   store $1, 0, align 4
; CHECK:   double* %3 = ptrshift $2, 16 * 0, 1 * 0
; CHECK:   double* %4 = ptrshift $2, 16 * 0, 1 * 8
; CHECK:   store %3, 1.0E+0, align 8
; CHECK:   store %4, 2.0E+0, align 8
; CHECK:   double* %5 = ptrshift $2, 16 * 0, 1 * 0
; CHECK:   double %6 = load %5, align 8
; CHECK:   double* %7 = ptrshift $2, 16 * 0, 1 * 8
; CHECK:   double %8 = load %7, align 8
; CHECK:   si32 %9 = fptosi %6
; CHECK:   return %9
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
!1 = !DIFile(filename: "complex.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "c", scope: !8, file: !1, line: 4, type: !13)
!13 = !DIBasicType(name: "complex", size: 128, encoding: DW_ATE_complex_float)
!14 = !DILocation(line: 4, column: 18, scope: !8)
!15 = !DILocation(line: 5, column: 16, scope: !8)
!16 = !DILocation(line: 5, column: 10, scope: !8)
!17 = !DILocation(line: 5, column: 3, scope: !8)
