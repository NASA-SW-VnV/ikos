; ModuleID = 'shufflevector.pp.bc'
source_filename = "shufflevector.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !16 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !23, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.value(metadata i8** %1, metadata !25, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.value(metadata <4 x float> <float 1.000000e+00, float 2.000000e+00, float 3.000000e+00, float 4.000000e+00>, metadata !26, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.value(metadata <4 x float> <float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00>, metadata !26, metadata !DIExpression()), !dbg !24
  call void @printv(<4 x float> <float 4.000000e+00, float 3.000000e+00, float 2.000000e+00, float 1.000000e+00>) #3, !dbg !27
  ret i32 0, !dbg !28
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   call @printv(<4.0E+0, 3.0E+0, 2.0E+0, 1.0E+0>)
; CHECK:   return 0
; CHECK: }
; CHECK: }

declare void @printv(<4 x float>) local_unnamed_addr #1
; CHECK: declare void @printv(<4 x float>)

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!11, !12, !13, !14}
!llvm.ident = !{!15}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "shufflevector.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4, !10}
!4 = !DIDerivedType(tag: DW_TAG_typedef, name: "__m128", file: !5, line: 17, baseType: !6)
!5 = !DIFile(filename: "Homebrew/Cellar/llvm/9.0.0_1/lib/clang/9.0.0/include/xmmintrin.h", directory: "/Users/marthaud")
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 128, flags: DIFlagVector, elements: !8)
!7 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!8 = !{!9}
!9 = !DISubrange(count: 4)
!10 = !DIDerivedType(tag: DW_TAG_typedef, name: "__v4sf", file: !5, line: 16, baseType: !6)
!11 = !{i32 2, !"Dwarf Version", i32 4}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = !{i32 7, !"PIC Level", i32 2}
!15 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!16 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 5, type: !17, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!17 = !DISubroutineType(types: !18)
!18 = !{!19, !19, !20}
!19 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !21, size: 64)
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !22, size: 64)
!22 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!23 = !DILocalVariable(name: "argc", arg: 1, scope: !16, file: !1, line: 5, type: !19)
!24 = !DILocation(line: 0, scope: !16)
!25 = !DILocalVariable(name: "argv", arg: 2, scope: !16, file: !1, line: 5, type: !20)
!26 = !DILocalVariable(name: "m", scope: !16, file: !1, line: 6, type: !4)
!27 = !DILocation(line: 8, column: 3, scope: !16)
!28 = !DILocation(line: 9, column: 1, scope: !16)
