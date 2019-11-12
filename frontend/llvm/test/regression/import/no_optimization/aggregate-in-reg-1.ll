; ModuleID = 'aggregate-in-reg-1.pp.bc'
source_filename = "aggregate-in-reg-1.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%class.Foo = type { %class.Vector3 }
%class.Vector3 = type { float, float, float }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo*) #3 align 2 !dbg !59 {
  %2 = alloca %class.Vector3, align 4
  %3 = alloca %class.Foo*, align 8
  %4 = alloca { <2 x float>, float }, align 8
  store %class.Foo* %0, %class.Foo** %3, align 8
  call void @llvm.dbg.declare(metadata %class.Foo** %3, metadata !60, metadata !DIExpression()), !dbg !61
  %5 = load %class.Foo*, %class.Foo** %3, align 8
  %6 = getelementptr inbounds %class.Foo, %class.Foo* %5, i32 0, i32 0, !dbg !62
  %7 = bitcast %class.Vector3* %2 to i8*, !dbg !62
  %8 = bitcast %class.Vector3* %6 to i8*, !dbg !62
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %7, i8* align 4 %8, i64 12, i1 false), !dbg !62
  %9 = bitcast { <2 x float>, float }* %4 to i8*, !dbg !63
  %10 = bitcast %class.Vector3* %2 to i8*, !dbg !63
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %9, i8* align 4 %10, i64 12, i1 false), !dbg !63
  %11 = load { <2 x float>, float }, { <2 x float>, float }* %4, align 8, !dbg !63
  ret { <2 x float>, float } %11, !dbg !63
}
; CHECK: define {0: <2 x float>, 8: float} @_ZN3Foo9get_coordEv({0: {0: float, 4: float, 8: float}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: float, 4: float, 8: float}* $2 = allocate {0: float, 4: float, 8: float}, 1, align 4
; CHECK:   {0: {0: float, 4: float, 8: float}}** $3 = allocate {0: {0: float, 4: float, 8: float}}*, 1, align 8
; CHECK:   {0: <2 x float>, 8: float}* $4 = allocate {0: <2 x float>, 8: float}, 1, align 8
; CHECK:   store $3, %1, align 8
; CHECK:   {0: {0: float, 4: float, 8: float}}** %5 = bitcast $3
; CHECK:   {0: {0: float, 4: float, 8: float}}* %6 = load %5, align 8
; CHECK:   {0: float, 4: float, 8: float}* %7 = ptrshift %6, 12 * 0, 1 * 0
; CHECK:   si8* %8 = bitcast $2
; CHECK:   si8* %9 = bitcast %7
; CHECK:   call @ar.memcpy(%8, %9, 12, 4, 4, 0)
; CHECK:   si8* %10 = bitcast $4
; CHECK:   si8* %11 = bitcast $2
; CHECK:   call @ar.memcpy(%10, %11, 12, 8, 4, 0)
; CHECK:   {0: <2 x float>, 8: float} %12 = load $4, align 8
; CHECK:   return %12
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN3FooC1Efff(%class.Foo*, float, float, float) unnamed_addr #2 align 2 !dbg !47 {
  %5 = alloca %class.Foo*, align 8
  %6 = alloca float, align 4
  %7 = alloca float, align 4
  %8 = alloca float, align 4
  store %class.Foo* %0, %class.Foo** %5, align 8
  call void @llvm.dbg.declare(metadata %class.Foo** %5, metadata !48, metadata !DIExpression()), !dbg !50
  store float %1, float* %6, align 4
  call void @llvm.dbg.declare(metadata float* %6, metadata !51, metadata !DIExpression()), !dbg !52
  store float %2, float* %7, align 4
  call void @llvm.dbg.declare(metadata float* %7, metadata !53, metadata !DIExpression()), !dbg !54
  store float %3, float* %8, align 4
  call void @llvm.dbg.declare(metadata float* %8, metadata !55, metadata !DIExpression()), !dbg !56
  %9 = load %class.Foo*, %class.Foo** %5, align 8
  %10 = load float, float* %6, align 4, !dbg !57
  %11 = load float, float* %7, align 4, !dbg !57
  %12 = load float, float* %8, align 4, !dbg !57
  call void @_ZN3FooC2Efff(%class.Foo* %9, float %10, float %11, float %12), !dbg !57
  ret void, !dbg !58
}
; CHECK: define void @_ZN3FooC1Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: float, 4: float, 8: float}}** $5 = allocate {0: {0: float, 4: float, 8: float}}*, 1, align 8
; CHECK:   float* $6 = allocate float, 1, align 4
; CHECK:   float* $7 = allocate float, 1, align 4
; CHECK:   float* $8 = allocate float, 1, align 4
; CHECK:   store $5, %1, align 8
; CHECK:   store $6, %2, align 4
; CHECK:   store $7, %3, align 4
; CHECK:   store $8, %4, align 4
; CHECK:   {0: {0: float, 4: float, 8: float}}* %9 = load $5, align 8
; CHECK:   float %10 = load $6, align 4
; CHECK:   float %11 = load $7, align 4
; CHECK:   float %12 = load $8, align 4
; CHECK:   call @_ZN3FooC2Efff(%9, %10, %11, %12)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN3FooC2Efff(%class.Foo*, float, float, float) unnamed_addr #2 align 2 !dbg !64 {
  %5 = alloca %class.Foo*, align 8
  %6 = alloca float, align 4
  %7 = alloca float, align 4
  %8 = alloca float, align 4
  store %class.Foo* %0, %class.Foo** %5, align 8
  call void @llvm.dbg.declare(metadata %class.Foo** %5, metadata !65, metadata !DIExpression()), !dbg !66
  store float %1, float* %6, align 4
  call void @llvm.dbg.declare(metadata float* %6, metadata !67, metadata !DIExpression()), !dbg !68
  store float %2, float* %7, align 4
  call void @llvm.dbg.declare(metadata float* %7, metadata !69, metadata !DIExpression()), !dbg !70
  store float %3, float* %8, align 4
  call void @llvm.dbg.declare(metadata float* %8, metadata !71, metadata !DIExpression()), !dbg !72
  %9 = load %class.Foo*, %class.Foo** %5, align 8
  %10 = getelementptr inbounds %class.Foo, %class.Foo* %9, i32 0, i32 0, !dbg !73
  %11 = load float, float* %6, align 4, !dbg !74
  %12 = load float, float* %7, align 4, !dbg !75
  %13 = load float, float* %8, align 4, !dbg !76
  call void @_ZN7Vector3IfEC1Efff(%class.Vector3* %10, float %11, float %12, float %13), !dbg !77
  ret void, !dbg !78
}
; CHECK: define void @_ZN3FooC2Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: float, 4: float, 8: float}}** $5 = allocate {0: {0: float, 4: float, 8: float}}*, 1, align 8
; CHECK:   float* $6 = allocate float, 1, align 4
; CHECK:   float* $7 = allocate float, 1, align 4
; CHECK:   float* $8 = allocate float, 1, align 4
; CHECK:   store $5, %1, align 8
; CHECK:   store $6, %2, align 4
; CHECK:   store $7, %3, align 4
; CHECK:   store $8, %4, align 4
; CHECK:   {0: {0: float, 4: float, 8: float}}** %9 = bitcast $5
; CHECK:   {0: {0: float, 4: float, 8: float}}* %10 = load %9, align 8
; CHECK:   {0: float, 4: float, 8: float}* %11 = ptrshift %10, 12 * 0, 1 * 0
; CHECK:   float %12 = load $6, align 4
; CHECK:   float %13 = load $7, align 4
; CHECK:   float %14 = load $8, align 4
; CHECK:   call @_ZN7Vector3IfEC1Efff(%11, %12, %13, %14)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN7Vector3IfEC1Efff(%class.Vector3*, float, float, float) unnamed_addr #2 align 2 !dbg !79 {
  %5 = alloca %class.Vector3*, align 8
  %6 = alloca float, align 4
  %7 = alloca float, align 4
  %8 = alloca float, align 4
  store %class.Vector3* %0, %class.Vector3** %5, align 8
  call void @llvm.dbg.declare(metadata %class.Vector3** %5, metadata !80, metadata !DIExpression()), !dbg !82
  store float %1, float* %6, align 4
  call void @llvm.dbg.declare(metadata float* %6, metadata !83, metadata !DIExpression()), !dbg !84
  store float %2, float* %7, align 4
  call void @llvm.dbg.declare(metadata float* %7, metadata !85, metadata !DIExpression()), !dbg !86
  store float %3, float* %8, align 4
  call void @llvm.dbg.declare(metadata float* %8, metadata !87, metadata !DIExpression()), !dbg !88
  %9 = load %class.Vector3*, %class.Vector3** %5, align 8
  %10 = load float, float* %6, align 4, !dbg !89
  %11 = load float, float* %7, align 4, !dbg !89
  %12 = load float, float* %8, align 4, !dbg !89
  call void @_ZN7Vector3IfEC2Efff(%class.Vector3* %9, float %10, float %11, float %12), !dbg !89
  ret void, !dbg !90
}
; CHECK: define void @_ZN7Vector3IfEC1Efff({0: float, 4: float, 8: float}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: float, 4: float, 8: float}** $5 = allocate {0: float, 4: float, 8: float}*, 1, align 8
; CHECK:   float* $6 = allocate float, 1, align 4
; CHECK:   float* $7 = allocate float, 1, align 4
; CHECK:   float* $8 = allocate float, 1, align 4
; CHECK:   store $5, %1, align 8
; CHECK:   store $6, %2, align 4
; CHECK:   store $7, %3, align 4
; CHECK:   store $8, %4, align 4
; CHECK:   {0: float, 4: float, 8: float}* %9 = load $5, align 8
; CHECK:   float %10 = load $6, align 4
; CHECK:   float %11 = load $7, align 4
; CHECK:   float %12 = load $8, align 4
; CHECK:   call @_ZN7Vector3IfEC2Efff(%9, %10, %11, %12)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN7Vector3IfEC2Efff(%class.Vector3*, float, float, float) unnamed_addr #3 align 2 !dbg !91 {
  %5 = alloca %class.Vector3*, align 8
  %6 = alloca float, align 4
  %7 = alloca float, align 4
  %8 = alloca float, align 4
  store %class.Vector3* %0, %class.Vector3** %5, align 8
  call void @llvm.dbg.declare(metadata %class.Vector3** %5, metadata !92, metadata !DIExpression()), !dbg !93
  store float %1, float* %6, align 4
  call void @llvm.dbg.declare(metadata float* %6, metadata !94, metadata !DIExpression()), !dbg !95
  store float %2, float* %7, align 4
  call void @llvm.dbg.declare(metadata float* %7, metadata !96, metadata !DIExpression()), !dbg !97
  store float %3, float* %8, align 4
  call void @llvm.dbg.declare(metadata float* %8, metadata !98, metadata !DIExpression()), !dbg !99
  %9 = load %class.Vector3*, %class.Vector3** %5, align 8
  %10 = getelementptr inbounds %class.Vector3, %class.Vector3* %9, i32 0, i32 0, !dbg !100
  %11 = load float, float* %6, align 4, !dbg !101
  store float %11, float* %10, align 4, !dbg !100
  %12 = getelementptr inbounds %class.Vector3, %class.Vector3* %9, i32 0, i32 1, !dbg !102
  %13 = load float, float* %7, align 4, !dbg !103
  store float %13, float* %12, align 4, !dbg !102
  %14 = getelementptr inbounds %class.Vector3, %class.Vector3* %9, i32 0, i32 2, !dbg !104
  %15 = load float, float* %8, align 4, !dbg !105
  store float %15, float* %14, align 4, !dbg !104
  ret void, !dbg !106
}
; CHECK: define void @_ZN7Vector3IfEC2Efff({0: float, 4: float, 8: float}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: float, 4: float, 8: float}** $5 = allocate {0: float, 4: float, 8: float}*, 1, align 8
; CHECK:   float* $6 = allocate float, 1, align 4
; CHECK:   float* $7 = allocate float, 1, align 4
; CHECK:   float* $8 = allocate float, 1, align 4
; CHECK:   store $5, %1, align 8
; CHECK:   store $6, %2, align 4
; CHECK:   store $7, %3, align 4
; CHECK:   store $8, %4, align 4
; CHECK:   {0: float, 4: float, 8: float}** %9 = bitcast $5
; CHECK:   {0: float, 4: float, 8: float}* %10 = load %9, align 8
; CHECK:   float* %11 = ptrshift %10, 12 * 0, 1 * 0
; CHECK:   float %12 = load $6, align 4
; CHECK:   store %11, %12, align 4
; CHECK:   float* %13 = ptrshift %10, 12 * 0, 1 * 4
; CHECK:   float %14 = load $7, align 4
; CHECK:   store %13, %14, align 4
; CHECK:   float* %15 = ptrshift %10, 12 * 0, 1 * 8
; CHECK:   float %16 = load $8, align 4
; CHECK:   store %15, %16, align 4
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #4
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !8 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca %class.Foo, align 4
  %7 = alloca %class.Vector3, align 4
  %8 = alloca { <2 x float>, float }, align 8
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !15, metadata !DIExpression()), !dbg !16
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.declare(metadata %class.Foo* %6, metadata !19, metadata !DIExpression()), !dbg !42
  call void @_ZN3FooC1Efff(%class.Foo* %6, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00), !dbg !42
  call void @llvm.dbg.declare(metadata %class.Vector3* %7, metadata !43, metadata !DIExpression()), !dbg !44
  %9 = call { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo* %6), !dbg !45
  store { <2 x float>, float } %9, { <2 x float>, float }* %8, align 8, !dbg !45
  %10 = bitcast { <2 x float>, float }* %8 to i8*, !dbg !45
  %11 = bitcast %class.Vector3* %7 to i8*, !dbg !45
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %11, i8* align 8 %10, i64 12, i1 false), !dbg !45
  ret i32 0, !dbg !46
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si8*** $5 = allocate si8**, 1, align 8
; CHECK:   {0: {0: float, 4: float, 8: float}}* $6 = allocate {0: {0: float, 4: float, 8: float}}, 1, align 4
; CHECK:   {0: float, 4: float, 8: float}* $7 = allocate {0: float, 4: float, 8: float}, 1, align 4
; CHECK:   {0: <2 x float>, 8: float}* $8 = allocate {0: <2 x float>, 8: float}, 1, align 8
; CHECK:   store $3, 0, align 4
; CHECK:   store $4, %1, align 4
; CHECK:   store $5, %2, align 8
; CHECK:   call @_ZN3FooC1Efff($6, 1.0E+0, 2.0E+0, 3.0E+0)
; CHECK:   {0: <2 x float>, 8: float} %9 = call @_ZN3Foo9get_coordEv($6)
; CHECK:   store $8, %9, align 8
; CHECK:   si8* %10 = bitcast $8
; CHECK:   si8* %11 = bitcast $7
; CHECK:   call @ar.memcpy(%11, %10, 12, 4, 8, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "aggregate-in-reg-1.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 19, type: !9, scopeLine: 19, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!15 = !DILocalVariable(name: "argc", arg: 1, scope: !8, file: !1, line: 19, type: !11)
!16 = !DILocation(line: 19, column: 14, scope: !8)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !8, file: !1, line: 19, type: !12)
!18 = !DILocation(line: 19, column: 26, scope: !8)
!19 = !DILocalVariable(name: "f", scope: !8, file: !1, line: 20, type: !20)
!20 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Foo", file: !1, line: 10, size: 96, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !21, identifier: "_ZTS3Foo")
!21 = !{!22, !35, !39}
!22 = !DIDerivedType(tag: DW_TAG_member, name: "coord", scope: !20, file: !1, line: 12, baseType: !23, size: 96)
!23 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Vector3<float>", file: !1, line: 2, size: 96, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !24, templateParams: !33, identifier: "_ZTS7Vector3IfE")
!24 = !{!25, !27, !28, !29}
!25 = !DIDerivedType(tag: DW_TAG_member, name: "_x", scope: !23, file: !1, line: 4, baseType: !26, size: 32)
!26 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "_y", scope: !23, file: !1, line: 4, baseType: !26, size: 32, offset: 32)
!28 = !DIDerivedType(tag: DW_TAG_member, name: "_z", scope: !23, file: !1, line: 4, baseType: !26, size: 32, offset: 64)
!29 = !DISubprogram(name: "Vector3", scope: !23, file: !1, line: 7, type: !30, scopeLine: 7, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!30 = !DISubroutineType(types: !31)
!31 = !{null, !32, !26, !26, !26}
!32 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!33 = !{!34}
!34 = !DITemplateTypeParameter(name: "T", type: !26)
!35 = !DISubprogram(name: "Foo", scope: !20, file: !1, line: 15, type: !36, scopeLine: 15, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!36 = !DISubroutineType(types: !37)
!37 = !{null, !38, !26, !26, !26}
!38 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!39 = !DISubprogram(name: "get_coord", linkageName: "_ZN3Foo9get_coordEv", scope: !20, file: !1, line: 16, type: !40, scopeLine: 16, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!40 = !DISubroutineType(types: !41)
!41 = !{!23, !38}
!42 = !DILocation(line: 20, column: 7, scope: !8)
!43 = !DILocalVariable(name: "coord", scope: !8, file: !1, line: 21, type: !23)
!44 = !DILocation(line: 21, column: 20, scope: !8)
!45 = !DILocation(line: 21, column: 30, scope: !8)
!46 = !DILocation(line: 22, column: 3, scope: !8)
!47 = distinct !DISubprogram(name: "Foo", linkageName: "_ZN3FooC1Efff", scope: !20, file: !1, line: 15, type: !36, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !35, retainedNodes: !2)
!48 = !DILocalVariable(name: "this", arg: 1, scope: !47, type: !49, flags: DIFlagArtificial | DIFlagObjectPointer)
!49 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!50 = !DILocation(line: 0, scope: !47)
!51 = !DILocalVariable(name: "x", arg: 2, scope: !47, file: !1, line: 15, type: !26)
!52 = !DILocation(line: 15, column: 13, scope: !47)
!53 = !DILocalVariable(name: "y", arg: 3, scope: !47, file: !1, line: 15, type: !26)
!54 = !DILocation(line: 15, column: 22, scope: !47)
!55 = !DILocalVariable(name: "z", arg: 4, scope: !47, file: !1, line: 15, type: !26)
!56 = !DILocation(line: 15, column: 31, scope: !47)
!57 = !DILocation(line: 15, column: 69, scope: !47)
!58 = !DILocation(line: 15, column: 70, scope: !47)
!59 = distinct !DISubprogram(name: "get_coord", linkageName: "_ZN3Foo9get_coordEv", scope: !20, file: !1, line: 16, type: !40, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !39, retainedNodes: !2)
!60 = !DILocalVariable(name: "this", arg: 1, scope: !59, type: !49, flags: DIFlagArtificial | DIFlagObjectPointer)
!61 = !DILocation(line: 0, scope: !59)
!62 = !DILocation(line: 16, column: 41, scope: !59)
!63 = !DILocation(line: 16, column: 34, scope: !59)
!64 = distinct !DISubprogram(name: "Foo", linkageName: "_ZN3FooC2Efff", scope: !20, file: !1, line: 15, type: !36, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !35, retainedNodes: !2)
!65 = !DILocalVariable(name: "this", arg: 1, scope: !64, type: !49, flags: DIFlagArtificial | DIFlagObjectPointer)
!66 = !DILocation(line: 0, scope: !64)
!67 = !DILocalVariable(name: "x", arg: 2, scope: !64, file: !1, line: 15, type: !26)
!68 = !DILocation(line: 15, column: 13, scope: !64)
!69 = !DILocalVariable(name: "y", arg: 3, scope: !64, file: !1, line: 15, type: !26)
!70 = !DILocation(line: 15, column: 22, scope: !64)
!71 = !DILocalVariable(name: "z", arg: 4, scope: !64, file: !1, line: 15, type: !26)
!72 = !DILocation(line: 15, column: 31, scope: !64)
!73 = !DILocation(line: 15, column: 36, scope: !64)
!74 = !DILocation(line: 15, column: 59, scope: !64)
!75 = !DILocation(line: 15, column: 62, scope: !64)
!76 = !DILocation(line: 15, column: 65, scope: !64)
!77 = !DILocation(line: 15, column: 42, scope: !64)
!78 = !DILocation(line: 15, column: 70, scope: !64)
!79 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC1Efff", scope: !23, file: !1, line: 7, type: !30, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !29, retainedNodes: !2)
!80 = !DILocalVariable(name: "this", arg: 1, scope: !79, type: !81, flags: DIFlagArtificial | DIFlagObjectPointer)
!81 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!82 = !DILocation(line: 0, scope: !79)
!83 = !DILocalVariable(name: "x", arg: 2, scope: !79, file: !1, line: 7, type: !26)
!84 = !DILocation(line: 7, column: 18, scope: !79)
!85 = !DILocalVariable(name: "y", arg: 3, scope: !79, file: !1, line: 7, type: !26)
!86 = !DILocation(line: 7, column: 23, scope: !79)
!87 = !DILocalVariable(name: "z", arg: 4, scope: !79, file: !1, line: 7, type: !26)
!88 = !DILocation(line: 7, column: 28, scope: !79)
!89 = !DILocation(line: 7, column: 53, scope: !79)
!90 = !DILocation(line: 7, column: 54, scope: !79)
!91 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC2Efff", scope: !23, file: !1, line: 7, type: !30, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !29, retainedNodes: !2)
!92 = !DILocalVariable(name: "this", arg: 1, scope: !91, type: !81, flags: DIFlagArtificial | DIFlagObjectPointer)
!93 = !DILocation(line: 0, scope: !91)
!94 = !DILocalVariable(name: "x", arg: 2, scope: !91, file: !1, line: 7, type: !26)
!95 = !DILocation(line: 7, column: 18, scope: !91)
!96 = !DILocalVariable(name: "y", arg: 3, scope: !91, file: !1, line: 7, type: !26)
!97 = !DILocation(line: 7, column: 23, scope: !91)
!98 = !DILocalVariable(name: "z", arg: 4, scope: !91, file: !1, line: 7, type: !26)
!99 = !DILocation(line: 7, column: 28, scope: !91)
!100 = !DILocation(line: 7, column: 33, scope: !91)
!101 = !DILocation(line: 7, column: 36, scope: !91)
!102 = !DILocation(line: 7, column: 40, scope: !91)
!103 = !DILocation(line: 7, column: 43, scope: !91)
!104 = !DILocation(line: 7, column: 47, scope: !91)
!105 = !DILocation(line: 7, column: 50, scope: !91)
!106 = !DILocation(line: 7, column: 54, scope: !91)
