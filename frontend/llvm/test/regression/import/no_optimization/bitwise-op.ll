; ModuleID = 'bitwise-op.pp.bc'
source_filename = "bitwise-op.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !8 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %1, metadata !12, metadata !DIExpression()), !dbg !13
  store i32 3, i32* %1, align 4, !dbg !13
  call void @llvm.dbg.declare(metadata i32* %2, metadata !14, metadata !DIExpression()), !dbg !15
  store i32 5, i32* %2, align 4, !dbg !15
  call void @llvm.dbg.declare(metadata i32* %3, metadata !16, metadata !DIExpression()), !dbg !17
  %4 = load i32, i32* %1, align 4, !dbg !18
  %5 = load i32, i32* %2, align 4, !dbg !19
  %6 = or i32 %4, %5, !dbg !20
  store i32 %6, i32* %3, align 4, !dbg !17
  ret i32 0, !dbg !21
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   si32* $2 = allocate si32, 1, align 4
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   store $1, 3, align 4
; CHECK:   store $2, 5, align 4
; CHECK:   ui32* %4 = bitcast $1
; CHECK:   ui32 %5 = load %4, align 4
; CHECK:   ui32* %6 = bitcast $2
; CHECK:   ui32 %7 = load %6, align 4
; CHECK:   ui32 %8 = %5 uor %7
; CHECK:   si32 %9 = bitcast %8
; CHECK:   store $3, %9, align 4
; CHECK:   return 0
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
!1 = !DIFile(filename: "bitwise-op.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!12 = !DILocalVariable(name: "x", scope: !8, file: !1, line: 2, type: !11)
!13 = !DILocation(line: 2, column: 7, scope: !8)
!14 = !DILocalVariable(name: "y", scope: !8, file: !1, line: 2, type: !11)
!15 = !DILocation(line: 2, column: 14, scope: !8)
!16 = !DILocalVariable(name: "z", scope: !8, file: !1, line: 3, type: !11)
!17 = !DILocation(line: 3, column: 7, scope: !8)
!18 = !DILocation(line: 3, column: 11, scope: !8)
!19 = !DILocation(line: 3, column: 15, scope: !8)
!20 = !DILocation(line: 3, column: 13, scope: !8)
!21 = !DILocation(line: 4, column: 1, scope: !8)
