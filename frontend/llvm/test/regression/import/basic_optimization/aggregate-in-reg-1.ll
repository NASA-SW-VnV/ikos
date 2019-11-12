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
define linkonce_odr { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo*) #3 align 2 !dbg !55 {
  %2 = alloca %class.Vector3, align 4
  %3 = alloca { <2 x float>, float }, align 8
  call void @llvm.dbg.value(metadata %class.Foo* %0, metadata !56, metadata !DIExpression()), !dbg !57
  %4 = getelementptr inbounds %class.Foo, %class.Foo* %0, i32 0, i32 0, !dbg !58
  %5 = bitcast %class.Vector3* %2 to i8*, !dbg !58
  %6 = bitcast %class.Vector3* %4 to i8*, !dbg !58
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %5, i8* align 4 %6, i64 12, i1 false), !dbg !58
  %7 = bitcast { <2 x float>, float }* %3 to i8*, !dbg !59
  %8 = bitcast %class.Vector3* %2 to i8*, !dbg !59
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %7, i8* align 4 %8, i64 12, i1 false), !dbg !59
  %9 = load { <2 x float>, float }, { <2 x float>, float }* %3, align 8, !dbg !59
  ret { <2 x float>, float } %9, !dbg !59
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
define linkonce_odr void @_ZN3FooC1Efff(%class.Foo*, float, float, float) unnamed_addr #2 align 2 !dbg !46 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, metadata !47, metadata !DIExpression()), !dbg !49
  call void @llvm.dbg.value(metadata float %1, metadata !50, metadata !DIExpression()), !dbg !49
  call void @llvm.dbg.value(metadata float %2, metadata !51, metadata !DIExpression()), !dbg !49
  call void @llvm.dbg.value(metadata float %3, metadata !52, metadata !DIExpression()), !dbg !49
  call void @_ZN3FooC2Efff(%class.Foo* %0, float %1, float %2, float %3), !dbg !53
  ret void, !dbg !54
}
; CHECK: define void @_ZN3FooC1Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN3FooC2Efff(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN3FooC2Efff(%class.Foo*, float, float, float) unnamed_addr #2 align 2 !dbg !60 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, metadata !61, metadata !DIExpression()), !dbg !62
  call void @llvm.dbg.value(metadata float %1, metadata !63, metadata !DIExpression()), !dbg !62
  call void @llvm.dbg.value(metadata float %2, metadata !64, metadata !DIExpression()), !dbg !62
  call void @llvm.dbg.value(metadata float %3, metadata !65, metadata !DIExpression()), !dbg !62
  %5 = getelementptr inbounds %class.Foo, %class.Foo* %0, i32 0, i32 0, !dbg !66
  call void @_ZN7Vector3IfEC1Efff(%class.Vector3* %5, float %1, float %2, float %3), !dbg !67
  ret void, !dbg !68
}
; CHECK: define void @_ZN3FooC2Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: float, 4: float, 8: float}* %5 = ptrshift %1, 12 * 0, 1 * 0
; CHECK:   call @_ZN7Vector3IfEC1Efff(%5, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN7Vector3IfEC1Efff(%class.Vector3*, float, float, float) unnamed_addr #2 align 2 !dbg !69 {
  call void @llvm.dbg.value(metadata %class.Vector3* %0, metadata !70, metadata !DIExpression()), !dbg !72
  call void @llvm.dbg.value(metadata float %1, metadata !73, metadata !DIExpression()), !dbg !72
  call void @llvm.dbg.value(metadata float %2, metadata !74, metadata !DIExpression()), !dbg !72
  call void @llvm.dbg.value(metadata float %3, metadata !75, metadata !DIExpression()), !dbg !72
  call void @_ZN7Vector3IfEC2Efff(%class.Vector3* %0, float %1, float %2, float %3), !dbg !76
  ret void, !dbg !77
}
; CHECK: define void @_ZN7Vector3IfEC1Efff({0: float, 4: float, 8: float}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN7Vector3IfEC2Efff(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN7Vector3IfEC2Efff(%class.Vector3*, float, float, float) unnamed_addr #3 align 2 !dbg !78 {
  call void @llvm.dbg.value(metadata %class.Vector3* %0, metadata !79, metadata !DIExpression()), !dbg !80
  call void @llvm.dbg.value(metadata float %1, metadata !81, metadata !DIExpression()), !dbg !80
  call void @llvm.dbg.value(metadata float %2, metadata !82, metadata !DIExpression()), !dbg !80
  call void @llvm.dbg.value(metadata float %3, metadata !83, metadata !DIExpression()), !dbg !80
  %5 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i32 0, i32 0, !dbg !84
  store float %1, float* %5, align 4, !dbg !84
  %6 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i32 0, i32 1, !dbg !85
  store float %2, float* %6, align 4, !dbg !85
  %7 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i32 0, i32 2, !dbg !86
  store float %3, float* %7, align 4, !dbg !86
  ret void, !dbg !87
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
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #4
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !8 {
  %3 = alloca %class.Foo, align 4
  %4 = alloca %class.Vector3, align 4
  %5 = alloca { <2 x float>, float }, align 8
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, metadata !17, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.declare(metadata %class.Foo* %3, metadata !18, metadata !DIExpression()), !dbg !41
  call void @_ZN3FooC1Efff(%class.Foo* %3, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00), !dbg !41
  call void @llvm.dbg.declare(metadata %class.Vector3* %4, metadata !42, metadata !DIExpression()), !dbg !43
  %6 = call { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo* %3), !dbg !44
  store { <2 x float>, float } %6, { <2 x float>, float }* %5, align 8, !dbg !44
  %7 = bitcast { <2 x float>, float }* %5 to i8*, !dbg !44
  %8 = bitcast %class.Vector3* %4 to i8*, !dbg !44
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %8, i8* align 8 %7, i64 12, i1 false), !dbg !44
  ret i32 0, !dbg !45
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

attributes #0 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "aggregate-in-reg-1.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
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
!16 = !DILocation(line: 0, scope: !8)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !8, file: !1, line: 19, type: !12)
!18 = !DILocalVariable(name: "f", scope: !8, file: !1, line: 20, type: !19)
!19 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Foo", file: !1, line: 10, size: 96, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !20, identifier: "_ZTS3Foo")
!20 = !{!21, !34, !38}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "coord", scope: !19, file: !1, line: 12, baseType: !22, size: 96)
!22 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Vector3<float>", file: !1, line: 2, size: 96, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !23, templateParams: !32, identifier: "_ZTS7Vector3IfE")
!23 = !{!24, !26, !27, !28}
!24 = !DIDerivedType(tag: DW_TAG_member, name: "_x", scope: !22, file: !1, line: 4, baseType: !25, size: 32)
!25 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!26 = !DIDerivedType(tag: DW_TAG_member, name: "_y", scope: !22, file: !1, line: 4, baseType: !25, size: 32, offset: 32)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "_z", scope: !22, file: !1, line: 4, baseType: !25, size: 32, offset: 64)
!28 = !DISubprogram(name: "Vector3", scope: !22, file: !1, line: 7, type: !29, scopeLine: 7, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!29 = !DISubroutineType(types: !30)
!30 = !{null, !31, !25, !25, !25}
!31 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !22, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!32 = !{!33}
!33 = !DITemplateTypeParameter(name: "T", type: !25)
!34 = !DISubprogram(name: "Foo", scope: !19, file: !1, line: 15, type: !35, scopeLine: 15, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!35 = !DISubroutineType(types: !36)
!36 = !{null, !37, !25, !25, !25}
!37 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!38 = !DISubprogram(name: "get_coord", linkageName: "_ZN3Foo9get_coordEv", scope: !19, file: !1, line: 16, type: !39, scopeLine: 16, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!39 = !DISubroutineType(types: !40)
!40 = !{!22, !37}
!41 = !DILocation(line: 20, column: 7, scope: !8)
!42 = !DILocalVariable(name: "coord", scope: !8, file: !1, line: 21, type: !22)
!43 = !DILocation(line: 21, column: 20, scope: !8)
!44 = !DILocation(line: 21, column: 30, scope: !8)
!45 = !DILocation(line: 22, column: 3, scope: !8)
!46 = distinct !DISubprogram(name: "Foo", linkageName: "_ZN3FooC1Efff", scope: !19, file: !1, line: 15, type: !35, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !34, retainedNodes: !2)
!47 = !DILocalVariable(name: "this", arg: 1, scope: !46, type: !48, flags: DIFlagArtificial | DIFlagObjectPointer)
!48 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!49 = !DILocation(line: 0, scope: !46)
!50 = !DILocalVariable(name: "x", arg: 2, scope: !46, file: !1, line: 15, type: !25)
!51 = !DILocalVariable(name: "y", arg: 3, scope: !46, file: !1, line: 15, type: !25)
!52 = !DILocalVariable(name: "z", arg: 4, scope: !46, file: !1, line: 15, type: !25)
!53 = !DILocation(line: 15, column: 69, scope: !46)
!54 = !DILocation(line: 15, column: 70, scope: !46)
!55 = distinct !DISubprogram(name: "get_coord", linkageName: "_ZN3Foo9get_coordEv", scope: !19, file: !1, line: 16, type: !39, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !38, retainedNodes: !2)
!56 = !DILocalVariable(name: "this", arg: 1, scope: !55, type: !48, flags: DIFlagArtificial | DIFlagObjectPointer)
!57 = !DILocation(line: 0, scope: !55)
!58 = !DILocation(line: 16, column: 41, scope: !55)
!59 = !DILocation(line: 16, column: 34, scope: !55)
!60 = distinct !DISubprogram(name: "Foo", linkageName: "_ZN3FooC2Efff", scope: !19, file: !1, line: 15, type: !35, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !34, retainedNodes: !2)
!61 = !DILocalVariable(name: "this", arg: 1, scope: !60, type: !48, flags: DIFlagArtificial | DIFlagObjectPointer)
!62 = !DILocation(line: 0, scope: !60)
!63 = !DILocalVariable(name: "x", arg: 2, scope: !60, file: !1, line: 15, type: !25)
!64 = !DILocalVariable(name: "y", arg: 3, scope: !60, file: !1, line: 15, type: !25)
!65 = !DILocalVariable(name: "z", arg: 4, scope: !60, file: !1, line: 15, type: !25)
!66 = !DILocation(line: 15, column: 36, scope: !60)
!67 = !DILocation(line: 15, column: 42, scope: !60)
!68 = !DILocation(line: 15, column: 70, scope: !60)
!69 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC1Efff", scope: !22, file: !1, line: 7, type: !29, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !28, retainedNodes: !2)
!70 = !DILocalVariable(name: "this", arg: 1, scope: !69, type: !71, flags: DIFlagArtificial | DIFlagObjectPointer)
!71 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !22, size: 64)
!72 = !DILocation(line: 0, scope: !69)
!73 = !DILocalVariable(name: "x", arg: 2, scope: !69, file: !1, line: 7, type: !25)
!74 = !DILocalVariable(name: "y", arg: 3, scope: !69, file: !1, line: 7, type: !25)
!75 = !DILocalVariable(name: "z", arg: 4, scope: !69, file: !1, line: 7, type: !25)
!76 = !DILocation(line: 7, column: 53, scope: !69)
!77 = !DILocation(line: 7, column: 54, scope: !69)
!78 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC2Efff", scope: !22, file: !1, line: 7, type: !29, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !28, retainedNodes: !2)
!79 = !DILocalVariable(name: "this", arg: 1, scope: !78, type: !71, flags: DIFlagArtificial | DIFlagObjectPointer)
!80 = !DILocation(line: 0, scope: !78)
!81 = !DILocalVariable(name: "x", arg: 2, scope: !78, file: !1, line: 7, type: !25)
!82 = !DILocalVariable(name: "y", arg: 3, scope: !78, file: !1, line: 7, type: !25)
!83 = !DILocalVariable(name: "z", arg: 4, scope: !78, file: !1, line: 7, type: !25)
!84 = !DILocation(line: 7, column: 33, scope: !78)
!85 = !DILocation(line: 7, column: 40, scope: !78)
!86 = !DILocation(line: 7, column: 47, scope: !78)
!87 = !DILocation(line: 7, column: 54, scope: !78)
