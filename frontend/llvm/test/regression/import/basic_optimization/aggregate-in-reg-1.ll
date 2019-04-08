; ModuleID = 'aggregate-in-reg-1.pp.bc'
source_filename = "aggregate-in-reg-1.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

%class.Foo = type { %class.Vector3 }
%class.Vector3 = type { float, float, float }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo*) #3 align 2 !dbg !59 {
  %2 = alloca %class.Vector3, align 4
  %3 = alloca { <2 x float>, float }, align 8
  call void @llvm.dbg.value(metadata %class.Foo* %0, metadata !60, metadata !DIExpression()), !dbg !61
  %4 = getelementptr inbounds %class.Foo, %class.Foo* %0, i32 0, i32 0, !dbg !62
  %5 = bitcast %class.Vector3* %2 to i8*, !dbg !62
  %6 = bitcast %class.Vector3* %4 to i8*, !dbg !62
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %5, i8* align 4 %6, i64 12, i1 false), !dbg !62
  %7 = bitcast { <2 x float>, float }* %3 to i8*, !dbg !63
  %8 = bitcast %class.Vector3* %2 to i8*, !dbg !63
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %7, i8* align 4 %8, i64 12, i1 false), !dbg !63
  %9 = load { <2 x float>, float }, { <2 x float>, float }* %3, align 8, !dbg !63
  ret { <2 x float>, float } %9, !dbg !63
}
; CHECK: define {0: <2 x float>, 8: float} @_ZN3Foo9get_coordEv({0: {0: float, 4: float, 8: float}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: float, 4: float, 8: float}* $2 = allocate {0: float, 4: float, 8: float}, 1, align 4
; CHECK:   {0: <2 x float>, 8: float}* $3 = allocate {0: <2 x float>, 8: float}, 1, align 8
; CHECK:   {0: float, 4: float, 8: float}* %4 = ptrshift %1, 12 * 0, 1 * 0
; CHECK:   si8* %5 = bitcast $2
; CHECK:   si8* %6 = bitcast %4
; CHECK:   call @ar.memcpy(%5, %6, 12, 4, 4, 0)
; CHECK:   si8* %7 = bitcast $3
; CHECK:   si8* %8 = bitcast $2
; CHECK:   call @ar.memcpy(%7, %8, 12, 8, 4, 0)
; CHECK:   {0: <2 x float>, 8: float} %9 = load $3, align 8
; CHECK:   return %9
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN3FooC1Efff(%class.Foo*, float, float, float) unnamed_addr #2 align 2 !dbg !47 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, metadata !48, metadata !DIExpression()), !dbg !50
  call void @llvm.dbg.value(metadata float %1, metadata !51, metadata !DIExpression()), !dbg !52
  call void @llvm.dbg.value(metadata float %2, metadata !53, metadata !DIExpression()), !dbg !54
  call void @llvm.dbg.value(metadata float %3, metadata !55, metadata !DIExpression()), !dbg !56
  call void @_ZN3FooC2Efff(%class.Foo* %0, float %1, float %2, float %3), !dbg !57
  ret void, !dbg !58
}
; CHECK: define void @_ZN3FooC1Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN3FooC2Efff(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN3FooC2Efff(%class.Foo*, float, float, float) unnamed_addr #2 align 2 !dbg !64 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, metadata !65, metadata !DIExpression()), !dbg !66
  call void @llvm.dbg.value(metadata float %1, metadata !67, metadata !DIExpression()), !dbg !68
  call void @llvm.dbg.value(metadata float %2, metadata !69, metadata !DIExpression()), !dbg !70
  call void @llvm.dbg.value(metadata float %3, metadata !71, metadata !DIExpression()), !dbg !72
  %5 = getelementptr inbounds %class.Foo, %class.Foo* %0, i32 0, i32 0, !dbg !73
  call void @_ZN7Vector3IfEC1Efff(%class.Vector3* %5, float %1, float %2, float %3), !dbg !74
  ret void, !dbg !75
}
; CHECK: define void @_ZN3FooC2Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: float, 4: float, 8: float}* %5 = ptrshift %1, 12 * 0, 1 * 0
; CHECK:   call @_ZN7Vector3IfEC1Efff(%5, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN7Vector3IfEC1Efff(%class.Vector3*, float, float, float) unnamed_addr #2 align 2 !dbg !76 {
  call void @llvm.dbg.value(metadata %class.Vector3* %0, metadata !77, metadata !DIExpression()), !dbg !79
  call void @llvm.dbg.value(metadata float %1, metadata !80, metadata !DIExpression()), !dbg !81
  call void @llvm.dbg.value(metadata float %2, metadata !82, metadata !DIExpression()), !dbg !83
  call void @llvm.dbg.value(metadata float %3, metadata !84, metadata !DIExpression()), !dbg !85
  call void @_ZN7Vector3IfEC2Efff(%class.Vector3* %0, float %1, float %2, float %3), !dbg !86
  ret void, !dbg !87
}
; CHECK: define void @_ZN7Vector3IfEC1Efff({0: float, 4: float, 8: float}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN7Vector3IfEC2Efff(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN7Vector3IfEC2Efff(%class.Vector3*, float, float, float) unnamed_addr #3 align 2 !dbg !88 {
  call void @llvm.dbg.value(metadata %class.Vector3* %0, metadata !89, metadata !DIExpression()), !dbg !90
  call void @llvm.dbg.value(metadata float %1, metadata !91, metadata !DIExpression()), !dbg !92
  call void @llvm.dbg.value(metadata float %2, metadata !93, metadata !DIExpression()), !dbg !94
  call void @llvm.dbg.value(metadata float %3, metadata !95, metadata !DIExpression()), !dbg !96
  %5 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i32 0, i32 0, !dbg !97
  store float %1, float* %5, align 4, !dbg !97
  %6 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i32 0, i32 1, !dbg !98
  store float %2, float* %6, align 4, !dbg !98
  %7 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i32 0, i32 2, !dbg !99
  store float %3, float* %7, align 4, !dbg !99
  ret void, !dbg !100
}
; CHECK: define void @_ZN7Vector3IfEC2Efff({0: float, 4: float, 8: float}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   float* %5 = ptrshift %1, 12 * 0, 1 * 0
; CHECK:   store %5, %2, align 4
; CHECK:   float* %6 = ptrshift %1, 12 * 0, 1 * 4
; CHECK:   store %6, %3, align 4
; CHECK:   float* %7 = ptrshift %1, 12 * 0, 1 * 8
; CHECK:   store %7, %4, align 4
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #4
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !8 {
  %3 = alloca %class.Foo, align 4
  %4 = alloca %class.Vector3, align 4
  %5 = alloca { <2 x float>, float }, align 8
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.declare(metadata %class.Foo* %3, metadata !19, metadata !DIExpression()), !dbg !42
  call void @_ZN3FooC1Efff(%class.Foo* %3, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00), !dbg !42
  call void @llvm.dbg.declare(metadata %class.Vector3* %4, metadata !43, metadata !DIExpression()), !dbg !44
  %6 = call { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo* %3), !dbg !45
  store { <2 x float>, float } %6, { <2 x float>, float }* %5, align 8, !dbg !45
  %7 = bitcast { <2 x float>, float }* %5 to i8*, !dbg !45
  %8 = bitcast %class.Vector3* %4 to i8*, !dbg !45
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %8, i8* align 8 %7, i64 12, i1 false), !dbg !45
  ret i32 0, !dbg !46
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: float, 4: float, 8: float}}* $3 = allocate {0: {0: float, 4: float, 8: float}}, 1, align 4
; CHECK:   {0: float, 4: float, 8: float}* $4 = allocate {0: float, 4: float, 8: float}, 1, align 4
; CHECK:   {0: <2 x float>, 8: float}* $5 = allocate {0: <2 x float>, 8: float}, 1, align 8
; CHECK:   call @_ZN3FooC1Efff($3, 1.0E+0, 2.0E+0, 3.0E+0)
; CHECK:   {0: <2 x float>, 8: float} %6 = call @_ZN3Foo9get_coordEv($3)
; CHECK:   store $5, %6, align 8
; CHECK:   si8* %7 = bitcast $5
; CHECK:   si8* %8 = bitcast $4
; CHECK:   call @ar.memcpy(%8, %7, 12, 4, 8, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 8.0.0 (tags/RELEASE_800/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "aggregate-in-reg-1.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 8.0.0 (tags/RELEASE_800/final)"}
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
!20 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Foo", file: !1, line: 10, size: 96, flags: DIFlagTypePassByValue, elements: !21, identifier: "_ZTS3Foo")
!21 = !{!22, !35, !39}
!22 = !DIDerivedType(tag: DW_TAG_member, name: "coord", scope: !20, file: !1, line: 12, baseType: !23, size: 96)
!23 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Vector3<float>", file: !1, line: 2, size: 96, flags: DIFlagTypePassByValue, elements: !24, templateParams: !33, identifier: "_ZTS7Vector3IfE")
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
!74 = !DILocation(line: 15, column: 42, scope: !64)
!75 = !DILocation(line: 15, column: 70, scope: !64)
!76 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC1Efff", scope: !23, file: !1, line: 7, type: !30, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !29, retainedNodes: !2)
!77 = !DILocalVariable(name: "this", arg: 1, scope: !76, type: !78, flags: DIFlagArtificial | DIFlagObjectPointer)
!78 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!79 = !DILocation(line: 0, scope: !76)
!80 = !DILocalVariable(name: "x", arg: 2, scope: !76, file: !1, line: 7, type: !26)
!81 = !DILocation(line: 7, column: 18, scope: !76)
!82 = !DILocalVariable(name: "y", arg: 3, scope: !76, file: !1, line: 7, type: !26)
!83 = !DILocation(line: 7, column: 23, scope: !76)
!84 = !DILocalVariable(name: "z", arg: 4, scope: !76, file: !1, line: 7, type: !26)
!85 = !DILocation(line: 7, column: 28, scope: !76)
!86 = !DILocation(line: 7, column: 53, scope: !76)
!87 = !DILocation(line: 7, column: 54, scope: !76)
!88 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC2Efff", scope: !23, file: !1, line: 7, type: !30, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !29, retainedNodes: !2)
!89 = !DILocalVariable(name: "this", arg: 1, scope: !88, type: !78, flags: DIFlagArtificial | DIFlagObjectPointer)
!90 = !DILocation(line: 0, scope: !88)
!91 = !DILocalVariable(name: "x", arg: 2, scope: !88, file: !1, line: 7, type: !26)
!92 = !DILocation(line: 7, column: 18, scope: !88)
!93 = !DILocalVariable(name: "y", arg: 3, scope: !88, file: !1, line: 7, type: !26)
!94 = !DILocation(line: 7, column: 23, scope: !88)
!95 = !DILocalVariable(name: "z", arg: 4, scope: !88, file: !1, line: 7, type: !26)
!96 = !DILocation(line: 7, column: 28, scope: !88)
!97 = !DILocation(line: 7, column: 33, scope: !88)
!98 = !DILocation(line: 7, column: 40, scope: !88)
!99 = !DILocation(line: 7, column: 47, scope: !88)
!100 = !DILocation(line: 7, column: 54, scope: !88)
