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
  %3 = alloca float, align 4
  store float %0, float* %3, align 4
  call void @llvm.dbg.declare(metadata float* %3, metadata !22, metadata !DIExpression()), !dbg !23
  %4 = getelementptr inbounds %struct.line_t, %struct.line_t* %2, i32 0, i32 0, !dbg !24
  %5 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %4, i32 0, i32 0, !dbg !25
  store float 0.000000e+00, float* %5, align 4, !dbg !25
  %6 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %4, i32 0, i32 1, !dbg !25
  %7 = load float, float* %3, align 4, !dbg !26
  store float %7, float* %6, align 4, !dbg !25
  %8 = getelementptr inbounds %struct.line_t, %struct.line_t* %2, i32 0, i32 1, !dbg !24
  %9 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %8, i32 0, i32 0, !dbg !27
  store float 2.000000e+00, float* %9, align 4, !dbg !27
  %10 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %8, i32 0, i32 1, !dbg !27
  store float 0.000000e+00, float* %10, align 4, !dbg !27
  %11 = bitcast %struct.line_t* %2 to { <2 x float>, <2 x float> }*, !dbg !28
  %12 = load { <2 x float>, <2 x float> }, { <2 x float>, <2 x float> }* %11, align 4, !dbg !28
  ret { <2 x float>, <2 x float> } %12, !dbg !28
}
; CHECK: define {0: <2 x float>, 8: <2 x float>} @_Z1ff(float %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: float, 4: float}, 8: {0: float, 4: float}}* $2 = allocate {0: {0: float, 4: float}, 8: {0: float, 4: float}}, 1, align 4
; CHECK:   float* $3 = allocate float, 1, align 4
; CHECK:   store $3, %1, align 4
; CHECK:   {0: float, 4: float}* %4 = ptrshift $2, 16 * 0, 1 * 0
; CHECK:   float* %5 = ptrshift %4, 8 * 0, 1 * 0
; CHECK:   store %5, 0.0E+0, align 4
; CHECK:   float* %6 = ptrshift %4, 8 * 0, 1 * 4
; CHECK:   float %7 = load $3, align 4
; CHECK:   store %6, %7, align 4
; CHECK:   {0: float, 4: float}* %8 = ptrshift $2, 16 * 0, 1 * 8
; CHECK:   float* %9 = ptrshift %8, 8 * 0, 1 * 0
; CHECK:   store %9, 2.0E+0, align 4
; CHECK:   float* %10 = ptrshift %8, 8 * 0, 1 * 4
; CHECK:   store %10, 0.0E+0, align 4
; CHECK:   {0: <2 x float>, 8: <2 x float>}* %11 = bitcast $2
; CHECK:   {0: <2 x float>, 8: <2 x float>} %12 = load %11, align 4
; CHECK:   return %12
; CHECK: }
; CHECK: }

declare i32 @printf(i8*, ...) #3
; CHECK: declare si32 @ar.libc.printf(si8*, ...)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main() #2 !dbg !29 {
  %1 = alloca i32, align 4
  %2 = alloca %struct.line_t, align 4
  store i32 0, i32* %1, align 4
  %3 = call { <2 x float>, <2 x float> } @_Z1ff(float 2.000000e+00), !dbg !33
  %4 = bitcast %struct.line_t* %2 to { <2 x float>, <2 x float> }*, !dbg !33
  %5 = getelementptr inbounds { <2 x float>, <2 x float> }, { <2 x float>, <2 x float> }* %4, i32 0, i32 0, !dbg !33
  %6 = extractvalue { <2 x float>, <2 x float> } %3, 0, !dbg !33
  store <2 x float> %6, <2 x float>* %5, align 4, !dbg !33
  %7 = getelementptr inbounds { <2 x float>, <2 x float> }, { <2 x float>, <2 x float> }* %4, i32 0, i32 1, !dbg !33
  %8 = extractvalue { <2 x float>, <2 x float> } %3, 1, !dbg !33
  store <2 x float> %8, <2 x float>* %7, align 4, !dbg !33
  %9 = getelementptr inbounds %struct.line_t, %struct.line_t* %2, i32 0, i32 0, !dbg !34
  %10 = getelementptr inbounds %struct.pos_t, %struct.pos_t* %9, i32 0, i32 1, !dbg !35
  %11 = load float, float* %10, align 4, !dbg !35
  %12 = fpext float %11 to double, !dbg !33
  %13 = getelementptr inbounds [4 x i8], [4 x i8]* @.str, i64 0, i64 0, !dbg !36
  %14 = call i32 (i8*, ...) @printf(i8* %13, double %12), !dbg !36
  ret i32 0, !dbg !37
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   {0: {0: float, 4: float}, 8: {0: float, 4: float}}* $2 = allocate {0: {0: float, 4: float}, 8: {0: float, 4: float}}, 1, align 4
; CHECK:   store $1, 0, align 4
; CHECK:   {0: <2 x float>, 8: <2 x float>} %3 = call @_Z1ff(2.0E+0)
; CHECK:   {0: <2 x float>, 8: <2 x float>}* %4 = bitcast $2
; CHECK:   <2 x float>* %5 = ptrshift %4, 16 * 0, 1 * 0
; CHECK:   <2 x float> %6 = extractelement %3, 0
; CHECK:   store %5, %6, align 4
; CHECK:   <2 x float>* %7 = ptrshift %4, 16 * 0, 1 * 8
; CHECK:   <2 x float> %8 = extractelement %3, 8
; CHECK:   store %7, %8, align 4
; CHECK:   {0: float, 4: float}* %9 = ptrshift $2, 16 * 0, 1 * 0
; CHECK:   float* %10 = ptrshift %9, 8 * 0, 1 * 4
; CHECK:   float %11 = load %10, align 4
; CHECK:   double %12 = fpext %11
; CHECK:   si8* %13 = ptrshift @.str, 4 * 0, 1 * 0
; CHECK:   si32 %14 = call @ar.libc.printf(%13, %12)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "aggregate-in-reg-2.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!23 = !DILocation(line: 12, column: 16, scope: !8)
!24 = !DILocation(line: 13, column: 10, scope: !8)
!25 = !DILocation(line: 13, column: 11, scope: !8)
!26 = !DILocation(line: 13, column: 17, scope: !8)
!27 = !DILocation(line: 13, column: 21, scope: !8)
!28 = !DILocation(line: 13, column: 3, scope: !8)
!29 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 16, type: !30, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!30 = !DISubroutineType(types: !31)
!31 = !{!32}
!32 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!33 = !DILocation(line: 17, column: 18, scope: !29)
!34 = !DILocation(line: 17, column: 25, scope: !29)
!35 = !DILocation(line: 17, column: 31, scope: !29)
!36 = !DILocation(line: 17, column: 3, scope: !29)
!37 = !DILocation(line: 18, column: 3, scope: !29)
