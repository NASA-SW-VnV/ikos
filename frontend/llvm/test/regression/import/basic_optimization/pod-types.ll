; ModuleID = 'pod-types.pp.bc'
source_filename = "pod-types.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

@b = common global i8 0, align 1, !dbg !19
; CHECK: define ui8* @b, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @b, 0, align 1
; CHECK: }
; CHECK: }

@d = common global double 0.000000e+00, align 8, !dbg !9
; CHECK: define double* @d, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @d, 0.0E+0, align 1
; CHECK: }
; CHECK: }

@f = common global float 0.000000e+00, align 4, !dbg !6
; CHECK: define float* @f, align 4, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @f, 0.0E+0, align 1
; CHECK: }
; CHECK: }

@i = common global i32 0, align 4, !dbg !0
; CHECK: define ui32* @i, align 4, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @i, 0, align 1
; CHECK: }
; CHECK: }

@p = common global i8* null, align 8, !dbg !12
; CHECK: define si8** @p, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @p, null, align 1
; CHECK: }
; CHECK: }

@q = common global i32* null, align 8, !dbg !15
; CHECK: define si32** @q, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @q, null, align 1
; CHECK: }
; CHECK: }

@tab = common global [10 x [12 x i16]] zeroinitializer, align 16, !dbg !22
; CHECK: define [10 x [12 x si16]]* @tab, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @tab, aggregate_zero, align 1
; CHECK: }
; CHECK: }

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #2
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define void @fun() #0 !dbg !35 {
  ret void, !dbg !38
}
; CHECK: define void @fun() {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !39 {
  %3 = alloca [10 x i32], align 16
  call void @llvm.dbg.value(metadata i32 %0, metadata !45, metadata !DIExpression()), !dbg !46
  call void @llvm.dbg.value(metadata i8** %1, metadata !47, metadata !DIExpression()), !dbg !46
  call void @llvm.dbg.declare(metadata [10 x i32]* %3, metadata !48, metadata !DIExpression()), !dbg !51
  %4 = bitcast [10 x i32]* %3 to i8*, !dbg !51
  call void @llvm.memset.p0i8.i64(i8* align 16 %4, i8 0, i64 40, i1 false), !dbg !51
  %5 = bitcast i8* %4 to [10 x i32]*, !dbg !51
  %6 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i32 0, i32 0, !dbg !51
  store i32 1, i32* %6, align 16, !dbg !51
  %7 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i32 0, i32 1, !dbg !51
  store i32 -1, i32* %7, align 4, !dbg !51
  %8 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i32 0, i32 2, !dbg !51
  store i32 255, i32* %8, align 8, !dbg !51
  %9 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i32 0, i32 3, !dbg !51
  store i32 42, i32* %9, align 4, !dbg !51
  ret i32 0, !dbg !52
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   [10 x si32]* $3 = allocate [10 x si32], 1, align 16
; CHECK:   si8* %4 = bitcast $3
; CHECK:   call @ar.memset(%4, 0, 40, 16, 0)
; CHECK:   [10 x si32]* %5 = bitcast %4
; CHECK:   si32* %6 = ptrshift %5, 40 * 0, 4 * 0
; CHECK:   store %6, 1, align 16
; CHECK:   si32* %7 = ptrshift %5, 40 * 0, 4 * 1
; CHECK:   store %7, -1, align 4
; CHECK:   si32* %8 = ptrshift %5, 40 * 0, 4 * 2
; CHECK:   store %8, 255, align 8
; CHECK:   si32* %9 = ptrshift %5, 40 * 0, 4 * 3
; CHECK:   store %9, 42, align 4
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!30, !31, !32, !33}
!llvm.ident = !{!34}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "i", scope: !2, file: !3, line: 1, type: !29, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "pod-types.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!4 = !{}
!5 = !{!0, !6, !9, !12, !15, !19, !22}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "f", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!8 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!9 = !DIGlobalVariableExpression(var: !10, expr: !DIExpression())
!10 = distinct !DIGlobalVariable(name: "d", scope: !2, file: !3, line: 5, type: !11, isLocal: false, isDefinition: true)
!11 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!12 = !DIGlobalVariableExpression(var: !13, expr: !DIExpression())
!13 = distinct !DIGlobalVariable(name: "p", scope: !2, file: !3, line: 7, type: !14, isLocal: false, isDefinition: true)
!14 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!15 = !DIGlobalVariableExpression(var: !16, expr: !DIExpression())
!16 = distinct !DIGlobalVariable(name: "q", scope: !2, file: !3, line: 9, type: !17, isLocal: false, isDefinition: true)
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64)
!18 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!19 = !DIGlobalVariableExpression(var: !20, expr: !DIExpression())
!20 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 11, type: !21, isLocal: false, isDefinition: true)
!21 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!22 = !DIGlobalVariableExpression(var: !23, expr: !DIExpression())
!23 = distinct !DIGlobalVariable(name: "tab", scope: !2, file: !3, line: 13, type: !24, isLocal: false, isDefinition: true)
!24 = !DICompositeType(tag: DW_TAG_array_type, baseType: !25, size: 1920, elements: !26)
!25 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!26 = !{!27, !28}
!27 = !DISubrange(count: 10)
!28 = !DISubrange(count: 12)
!29 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!30 = !{i32 2, !"Dwarf Version", i32 4}
!31 = !{i32 2, !"Debug Info Version", i32 3}
!32 = !{i32 1, !"wchar_size", i32 4}
!33 = !{i32 7, !"PIC Level", i32 2}
!34 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!35 = distinct !DISubprogram(name: "fun", scope: !3, file: !3, line: 15, type: !36, scopeLine: 15, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!36 = !DISubroutineType(types: !37)
!37 = !{null}
!38 = !DILocation(line: 15, column: 13, scope: !35)
!39 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 17, type: !40, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!40 = !DISubroutineType(types: !41)
!41 = !{!18, !18, !42}
!42 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !43, size: 64)
!43 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !44, size: 64)
!44 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!45 = !DILocalVariable(name: "argc", arg: 1, scope: !39, file: !3, line: 17, type: !18)
!46 = !DILocation(line: 0, scope: !39)
!47 = !DILocalVariable(name: "argv", arg: 2, scope: !39, file: !3, line: 17, type: !42)
!48 = !DILocalVariable(name: "xxx", scope: !39, file: !3, line: 18, type: !49)
!49 = !DICompositeType(tag: DW_TAG_array_type, baseType: !18, size: 320, elements: !50)
!50 = !{!27}
!51 = !DILocation(line: 18, column: 7, scope: !39)
!52 = !DILocation(line: 19, column: 3, scope: !39)
