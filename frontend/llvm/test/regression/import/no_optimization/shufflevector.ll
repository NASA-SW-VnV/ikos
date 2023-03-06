; ModuleID = 'shufflevector.pp.bc'
source_filename = "shufflevector.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !16 {
  %3 = alloca float, align 4
  %4 = alloca float, align 4
  %5 = alloca float, align 4
  %6 = alloca float, align 4
  %7 = alloca <4 x float>, align 16
  %8 = alloca i32, align 4
  %9 = alloca i8**, align 8
  %10 = alloca <4 x float>, align 16
  store i32 %0, i32* %8, align 4
  call void @llvm.dbg.declare(metadata i32* %8, metadata !23, metadata !DIExpression()), !dbg !24
  store i8** %1, i8*** %9, align 8
  call void @llvm.dbg.declare(metadata i8*** %9, metadata !25, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.declare(metadata <4 x float>* %10, metadata !27, metadata !DIExpression()), !dbg !28
  store float 4.000000e+00, float* %3, align 4, !dbg !29
  store float 3.000000e+00, float* %4, align 4, !dbg !29
  store float 2.000000e+00, float* %5, align 4, !dbg !29
  store float 1.000000e+00, float* %6, align 4, !dbg !29
  %11 = load float, float* %6, align 4, !dbg !29
  %12 = insertelement <4 x float> undef, float %11, i32 0, !dbg !29
  %13 = load float, float* %5, align 4, !dbg !29
  %14 = insertelement <4 x float> %12, float %13, i32 1, !dbg !29
  %15 = load float, float* %4, align 4, !dbg !29
  %16 = insertelement <4 x float> %14, float %15, i32 2, !dbg !29
  %17 = load float, float* %3, align 4, !dbg !29
  %18 = insertelement <4 x float> %16, float %17, i32 3, !dbg !29
  store <4 x float> %18, <4 x float>* %7, align 16, !dbg !29
  %19 = load <4 x float>, <4 x float>* %7, align 16, !dbg !29
  store <4 x float> %19, <4 x float>* %10, align 16, !dbg !28
  %20 = load <4 x float>, <4 x float>* %10, align 16, !dbg !30
  %21 = load <4 x float>, <4 x float>* %10, align 16, !dbg !30
  %22 = shufflevector <4 x float> %20, <4 x float> %21, <4 x i32> <i32 3, i32 2, i32 5, i32 4>, !dbg !30
  store <4 x float> %22, <4 x float>* %10, align 16, !dbg !31
  %23 = load <4 x float>, <4 x float>* %10, align 16, !dbg !32
  call void @printv(<4 x float> %23), !dbg !33
  ret i32 0, !dbg !34
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   float* $3 = allocate float, 1, align 4
; CHECK:   float* $4 = allocate float, 1, align 4
; CHECK:   float* $5 = allocate float, 1, align 4
; CHECK:   float* $6 = allocate float, 1, align 4
; CHECK:   <4 x float>* $7 = allocate <4 x float>, 1, align 16
; CHECK:   si32* $8 = allocate si32, 1, align 4
; CHECK:   si8*** $9 = allocate si8**, 1, align 8
; CHECK:   <4 x float>* $10 = allocate <4 x float>, 1, align 16
; CHECK:   store $8, %1, align 4
; CHECK:   store $9, %2, align 8
; CHECK:   store $3, 4.0E+0, align 4
; CHECK:   store $4, 3.0E+0, align 4
; CHECK:   store $5, 2.0E+0, align 4
; CHECK:   store $6, 1.0E+0, align 4
; CHECK:   float %11 = load $6, align 4
; CHECK:   <4 x float> %12 = insertelement undef, 0, %11
; CHECK:   float %13 = load $5, align 4
; CHECK:   <4 x float> %14 = insertelement %12, 4, %13
; CHECK:   float %15 = load $4, align 4
; CHECK:   <4 x float> %16 = insertelement %14, 8, %15
; CHECK:   float %17 = load $3, align 4
; CHECK:   <4 x float> %18 = insertelement %16, 12, %17
; CHECK:   store $7, %18, align 16
; CHECK:   <4 x float> %19 = load $7, align 16
; CHECK:   store $10, %19, align 16
; CHECK:   <4 x float> %20 = load $10, align 16
; CHECK:   <4 x float> %21 = load $10, align 16
; CHECK:   <4 x float> %22 = shufflevector %20, %21
; CHECK:   store $10, %22, align 16
; CHECK:   <4 x float> %23 = load $10, align 16
; CHECK:   call @printv(%23)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @printv(<4 x float>) #2
; CHECK: declare void @printv(<4 x float>)

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!11, !12, !13, !14}
!llvm.ident = !{!15}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "shufflevector.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{!4, !10}
!4 = !DIDerivedType(tag: DW_TAG_typedef, name: "__m128", file: !5, line: 17, baseType: !6)
!5 = !DIFile(filename: "Homebrew/Cellar/llvm/9.0.0/lib/clang/9.0.0/include/xmmintrin.h", directory: "/Users/marthaud")
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
!24 = !DILocation(line: 5, column: 14, scope: !16)
!25 = !DILocalVariable(name: "argv", arg: 2, scope: !16, file: !1, line: 5, type: !20)
!26 = !DILocation(line: 5, column: 27, scope: !16)
!27 = !DILocalVariable(name: "m", scope: !16, file: !1, line: 6, type: !4)
!28 = !DILocation(line: 6, column: 10, scope: !16)
!29 = !DILocation(line: 6, column: 14, scope: !16)
!30 = !DILocation(line: 7, column: 7, scope: !16)
!31 = !DILocation(line: 7, column: 5, scope: !16)
!32 = !DILocation(line: 8, column: 10, scope: !16)
!33 = !DILocation(line: 8, column: 3, scope: !16)
!34 = !DILocation(line: 9, column: 1, scope: !16)
