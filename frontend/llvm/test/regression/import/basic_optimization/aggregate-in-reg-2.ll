; ModuleID = 'aggregate-in-reg-2.pp.bc'
source_filename = "aggregate-in-reg-2.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.line_t = type { %struct.pos_t, %struct.pos_t }
%struct.pos_t = type { float, float }

@.str = private unnamed_addr constant [4 x i8] c"%f\0A\00", align 1
; CHECK: define [4 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [37, 102, 10, 0], align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define { <2 x float>, <2 x float> } @_Z1ff(float) #0 !dbg !8 {
  %2 = alloca %struct.line_t, align 4
  call void @llvm.dbg.value(metadata float %0, metadata !22, metadata !DIExpression()), !dbg !23
  %3 = getelementptr inbounds %struct.line_t, %struct.line_t* %2, i32 0, i32 0, !dbg !24
  %4 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %3, i32 0, i32 0, !dbg !25
  store float 0.000000e+00, float* %4, align 4, !dbg !25
  %5 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %3, i32 0, i32 1, !dbg !25
  store float %0, float* %5, align 4, !dbg !25
  %6 = getelementptr inbounds %struct.line_t, %struct.line_t* %2, i32 0, i32 1, !dbg !24
  %7 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %6, i32 0, i32 0, !dbg !26
  store float 2.000000e+00, float* %7, align 4, !dbg !26
  %8 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %6, i32 0, i32 1, !dbg !26
  store float 0.000000e+00, float* %8, align 4, !dbg !26
  %9 = bitcast %struct.line_t* %2 to { <2 x float>, <2 x float> }*, !dbg !27
  %10 = load { <2 x float>, <2 x float> }, { <2 x float>, <2 x float> }* %9, align 4, !dbg !27
  ret { <2 x float>, <2 x float> } %10, !dbg !27
}
; CHECK: define {0: <2 x float>, 8: <2 x float>} @_Z1ff(float %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: float, 4: float}, 8: {0: float, 4: float}}* $2 = allocate {0: {0: float, 4: float}, 8: {0: float, 4: float}}, 1, align 4
; CHECK:   {0: float, 4: float}* %3 = ptrshift $2, 16 * 0, 1 * 0
; CHECK:   float* %4 = ptrshift %3, 8 * 0, 1 * 0
; CHECK:   store %4, 0.0E+0, align 4
; CHECK:   float* %5 = ptrshift %3, 8 * 0, 1 * 4
; CHECK:   store %5, %1, align 4
; CHECK:   {0: float, 4: float}* %6 = ptrshift $2, 16 * 0, 1 * 8
; CHECK:   float* %7 = ptrshift %6, 8 * 0, 1 * 0
; CHECK:   store %7, 2.0E+0, align 4
; CHECK:   float* %8 = ptrshift %6, 8 * 0, 1 * 4
; CHECK:   store %8, 0.0E+0, align 4
; CHECK:   {0: <2 x float>, 8: <2 x float>}* %9 = bitcast $2
; CHECK:   {0: <2 x float>, 8: <2 x float>} %10 = load %9, align 4
; CHECK:   return %10
; CHECK: }
; CHECK: }

declare i32 @printf(i8*, ...) #2
; CHECK: declare si32 @ar.libc.printf(si8*, ...)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main() #1 !dbg !28 {
  %1 = alloca %struct.line_t, align 4
  %2 = call { <2 x float>, <2 x float> } @_Z1ff(float 2.000000e+00), !dbg !32
  %3 = bitcast %struct.line_t* %1 to { <2 x float>, <2 x float> }*, !dbg !32
  %4 = getelementptr inbounds { <2 x float>, <2 x float> }, { <2 x float>, <2 x float> }* %3, i32 0, i32 0, !dbg !32
  %5 = extractvalue { <2 x float>, <2 x float> } %2, 0, !dbg !32
  store <2 x float> %5, <2 x float>* %4, align 4, !dbg !32
  %6 = getelementptr inbounds { <2 x float>, <2 x float> }, { <2 x float>, <2 x float> }* %3, i32 0, i32 1, !dbg !32
  %7 = extractvalue { <2 x float>, <2 x float> } %2, 1, !dbg !32
  store <2 x float> %7, <2 x float>* %6, align 4, !dbg !32
  %8 = getelementptr inbounds %struct.line_t, %struct.line_t* %1, i32 0, i32 0, !dbg !33
  %9 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %8, i32 0, i32 1, !dbg !34
  %10 = load float, float* %9, align 4, !dbg !34
  %11 = fpext float %10 to double, !dbg !32
  %12 = getelementptr inbounds [4 x i8], [4 x i8]* @.str, i64 0, i64 0, !dbg !35
  %13 = call i32 (i8*, ...) @printf(i8* %12, double %11), !dbg !35
  ret i32 0, !dbg !36
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: float, 4: float}, 8: {0: float, 4: float}}* $1 = allocate {0: {0: float, 4: float}, 8: {0: float, 4: float}}, 1, align 4
; CHECK:   {0: <2 x float>, 8: <2 x float>} %2 = call @_Z1ff(2.0E+0)
; CHECK:   {0: <2 x float>, 8: <2 x float>}* %3 = bitcast $1
; CHECK:   <2 x float>* %4 = ptrshift %3, 16 * 0, 1 * 0
; CHECK:   <2 x float> %5 = extractelement %2, 0
; CHECK:   store %4, %5, align 4
; CHECK:   <2 x float>* %6 = ptrshift %3, 16 * 0, 1 * 8
; CHECK:   <2 x float> %7 = extractelement %2, 8
; CHECK:   store %6, %7, align 4
; CHECK:   {0: float, 4: float}* %8 = ptrshift $1, 16 * 0, 1 * 0
; CHECK:   float* %9 = ptrshift %8, 8 * 0, 1 * 4
; CHECK:   float %10 = load %9, align 4
; CHECK:   double %11 = fpext %10
; CHECK:   si8* %12 = ptrshift @.str, 4 * 0, 1 * 0
; CHECK:   si32 %13 = call @ar.libc.printf(%12, %11)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "aggregate-in-reg-2.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
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
!24 = !DILocation(line: 13, column: 10, scope: !8)
!25 = !DILocation(line: 13, column: 11, scope: !8)
!26 = !DILocation(line: 13, column: 21, scope: !8)
!27 = !DILocation(line: 13, column: 3, scope: !8)
!28 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 16, type: !29, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!29 = !DISubroutineType(types: !30)
!30 = !{!31}
!31 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!32 = !DILocation(line: 17, column: 18, scope: !28)
!33 = !DILocation(line: 17, column: 25, scope: !28)
!34 = !DILocation(line: 17, column: 31, scope: !28)
!35 = !DILocation(line: 17, column: 3, scope: !28)
!36 = !DILocation(line: 18, column: 3, scope: !28)
