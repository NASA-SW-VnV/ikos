; ModuleID = 'vector-4.pp.bc'
source_filename = "vector-4.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline nounwind ssp uwtable
define <2 x i64> @f(<2 x i64>) #0 !dbg !8 {
  %2 = alloca <2 x i64>, align 16
  store <2 x i64> %0, <2 x i64>* %2, align 16
  call void @llvm.dbg.declare(metadata <2 x i64>* %2, metadata !16, metadata !DIExpression()), !dbg !17
  %3 = load <2 x i64>, <2 x i64>* %2, align 16, !dbg !18
  %4 = bitcast <2 x i64> %3 to <4 x i32>, !dbg !18
  %5 = shufflevector <4 x i32> %4, <4 x i32> undef, <4 x i32> zeroinitializer, !dbg !19
  %6 = bitcast <4 x i32> %5 to <2 x i64>, !dbg !19
  ret <2 x i64> %6, !dbg !20
}
; CHECK: define <2 x si64> @f(<2 x si64> %1) {
; CHECK: #1 !entry !exit {
; CHECK:   <2 x si64>* $2 = allocate <2 x si64>, 1, align 16
; CHECK:   store $2, %1, align 16
; CHECK:   <2 x si64> %3 = load $2, align 16
; CHECK:   <4 x si32> %4 = bitcast %3
; CHECK:   <4 x si32> %5 = shufflevector %4, undef
; CHECK:   <2 x si64> %6 = bitcast %5
; CHECK:   return %6
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "vector-4.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11}
!11 = !DIDerivedType(tag: DW_TAG_typedef, name: "vector_t", file: !1, line: 1, baseType: !12)
!12 = !DICompositeType(tag: DW_TAG_array_type, baseType: !13, size: 128, flags: DIFlagVector, elements: !14)
!13 = !DIBasicType(name: "long int", size: 64, encoding: DW_ATE_signed)
!14 = !{!15}
!15 = !DISubrange(count: 2)
!16 = !DILocalVariable(name: "x", arg: 1, scope: !8, file: !1, line: 3, type: !11)
!17 = !DILocation(line: 3, column: 21, scope: !8)
!18 = !DILocation(line: 4, column: 32, scope: !8)
!19 = !DILocation(line: 4, column: 10, scope: !8)
!20 = !DILocation(line: 4, column: 3, scope: !8)
