; ModuleID = 'aggregate-in-reg-2.pp.bc'
source_filename = "aggregate-in-reg-2.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc { <2 x float>, <2 x float> } @_Z1ff(float) unnamed_addr #0 !dbg !8 {
  call void @llvm.dbg.value(metadata float %0, metadata !22, metadata !DIExpression()), !dbg !23
  %.sroa.0.4.vec.insert = insertelement <2 x float> <float 0.000000e+00, float undef>, float %0, i32 1, !dbg !24
  %.fca.0.insert = insertvalue { <2 x float>, <2 x float> } undef, <2 x float> %.sroa.0.4.vec.insert, 0, !dbg !25
  %.fca.1.insert = insertvalue { <2 x float>, <2 x float> } %.fca.0.insert, <2 x float> <float 2.000000e+00, float 0.000000e+00>, 1, !dbg !25
  ret { <2 x float>, <2 x float> } %.fca.1.insert, !dbg !25
}
; CHECK: define {0: <2 x float>, 8: <2 x float>} @_Z1ff(float %1) {
; CHECK: #1 !entry !exit {
; CHECK:   <2 x float> %.sroa.0.4.vec.insert = insertelement <0.0E+0, undef>, 4, %1
; CHECK:   {0: <2 x float>, 8: <2 x float>} %.fca.0.insert = insertelement undef, 0, %.sroa.0.4.vec.insert
; CHECK:   {0: <2 x float>, 8: <2 x float>} %.fca.1.insert = insertelement %.fca.0.insert, 8, <2.0E+0, 0.0E+0>
; CHECK:   return %.fca.1.insert
; CHECK: }
; CHECK: }

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main() local_unnamed_addr #1 !dbg !26 {
  %1 = call fastcc { <2 x float>, <2 x float> } @_Z1ff(float 2.000000e+00), !dbg !30
  ret i32 0, !dbg !31
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: <2 x float>, 8: <2 x float>} %1 = call @_Z1ff(2.0E+0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "aggregate-in-reg-2.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "f", linkageName: "_Z1ff", scope: !1, file: !1, line: 12, type: !9, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !19}
!11 = !DIDerivedType(tag: DW_TAG_typedef, name: "line_t", file: !1, line: 10, baseType: !12)
!12 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1, line: 8, size: 128, flags: DIFlagTypePassByValue, elements: !13, identifier: "_ZTS6line_t")
!13 = !{!14, !21}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "begin", scope: !12, file: !1, line: 9, baseType: !15, size: 64)
!15 = !DIDerivedType(tag: DW_TAG_typedef, name: "pos_t", file: !1, line: 6, baseType: !16)
!16 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1, line: 3, size: 64, flags: DIFlagTypePassByValue, elements: !17, identifier: "_ZTS5pos_t")
!17 = !{!18, !20}
!18 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !16, file: !1, line: 4, baseType: !19, size: 32)
!19 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!20 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !16, file: !1, line: 5, baseType: !19, size: 32, offset: 32)
!21 = !DIDerivedType(tag: DW_TAG_member, name: "end", scope: !12, file: !1, line: 9, baseType: !15, size: 64, offset: 64)
!22 = !DILocalVariable(name: "y", arg: 1, scope: !8, file: !1, line: 12, type: !19)
!23 = !DILocation(line: 0, scope: !8)
!24 = !DILocation(line: 13, column: 11, scope: !8)
!25 = !DILocation(line: 13, column: 3, scope: !8)
!26 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 16, type: !27, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!27 = !DISubroutineType(types: !28)
!28 = !{!29}
!29 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!30 = !DILocation(line: 17, column: 18, scope: !26)
!31 = !DILocation(line: 18, column: 3, scope: !26)
