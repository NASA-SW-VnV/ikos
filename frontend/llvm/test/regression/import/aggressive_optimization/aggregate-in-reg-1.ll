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
define internal fastcc { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo*) unnamed_addr #2 align 2 !dbg !57 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, metadata !58, metadata !DIExpression()), !dbg !59
  %.sroa.02.0..sroa_cast = bitcast %class.Foo* %0 to <2 x float>*, !dbg !60
  %.sroa.02.0.copyload = load <2 x float>, <2 x float>* %.sroa.02.0..sroa_cast, align 4, !dbg !60
  %.sroa.23.0..sroa_idx4 = getelementptr inbounds %class.Foo, %class.Foo* %0, i64 0, i32 0, i32 2, !dbg !60
  %.sroa.23.0.copyload = load float, float* %.sroa.23.0..sroa_idx4, align 4, !dbg !60
  %.fca.0.insert = insertvalue { <2 x float>, float } undef, <2 x float> %.sroa.02.0.copyload, 0, !dbg !61
  %.fca.1.insert = insertvalue { <2 x float>, float } %.fca.0.insert, float %.sroa.23.0.copyload, 1, !dbg !61
  ret { <2 x float>, float } %.fca.1.insert, !dbg !61
}
; CHECK: define {0: <2 x float>, 8: float} @_ZN3Foo9get_coordEv({0: {0: float, 4: float, 8: float}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   <2 x float>* %.sroa.02.0..sroa_cast = bitcast %1
; CHECK:   <2 x float> %.sroa.02.0.copyload = load %.sroa.02.0..sroa_cast, align 4
; CHECK:   float* %.sroa.23.0..sroa_idx4 = ptrshift %1, 12 * 0, 1 * 0, 1 * 8
; CHECK:   float %.sroa.23.0.copyload = load %.sroa.23.0..sroa_idx4, align 4
; CHECK:   {0: <2 x float>, 8: float} %.fca.0.insert = insertelement undef, 0, %.sroa.02.0.copyload
; CHECK:   {0: <2 x float>, 8: float} %.fca.1.insert = insertelement %.fca.0.insert, 8, %.sroa.23.0.copyload
; CHECK:   return %.fca.1.insert
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define internal fastcc void @_ZN3FooC1Efff(%class.Foo*, float, float, float) unnamed_addr #1 align 2 !dbg !45 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, metadata !46, metadata !DIExpression()), !dbg !48
  call void @llvm.dbg.value(metadata float %1, metadata !49, metadata !DIExpression()), !dbg !50
  call void @llvm.dbg.value(metadata float %2, metadata !51, metadata !DIExpression()), !dbg !52
  call void @llvm.dbg.value(metadata float %3, metadata !53, metadata !DIExpression()), !dbg !54
  call fastcc void @_ZN3FooC2Efff(%class.Foo* %0, float %1, float %2, float %3), !dbg !55
  ret void, !dbg !56
}
; CHECK: define void @_ZN3FooC1Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN3FooC2Efff(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define internal fastcc void @_ZN3FooC2Efff(%class.Foo*, float, float, float) unnamed_addr #1 align 2 !dbg !62 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, metadata !63, metadata !DIExpression()), !dbg !64
  call void @llvm.dbg.value(metadata float %1, metadata !65, metadata !DIExpression()), !dbg !66
  call void @llvm.dbg.value(metadata float %2, metadata !67, metadata !DIExpression()), !dbg !68
  call void @llvm.dbg.value(metadata float %3, metadata !69, metadata !DIExpression()), !dbg !70
  %5 = getelementptr inbounds %class.Foo, %class.Foo* %0, i64 0, i32 0, !dbg !71
  call fastcc void @_ZN7Vector3IfEC1Efff(%class.Vector3* %5, float %1, float %2, float %3), !dbg !72
  ret void, !dbg !73
}
; CHECK: define void @_ZN3FooC2Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: float, 4: float, 8: float}* %5 = ptrshift %1, 12 * 0, 1 * 0
; CHECK:   call @_ZN7Vector3IfEC1Efff(%5, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define internal fastcc void @_ZN7Vector3IfEC1Efff(%class.Vector3*, float, float, float) unnamed_addr #1 align 2 !dbg !74 {
  call void @llvm.dbg.value(metadata %class.Vector3* %0, metadata !75, metadata !DIExpression()), !dbg !77
  call void @llvm.dbg.value(metadata float %1, metadata !78, metadata !DIExpression()), !dbg !79
  call void @llvm.dbg.value(metadata float %2, metadata !80, metadata !DIExpression()), !dbg !81
  call void @llvm.dbg.value(metadata float %3, metadata !82, metadata !DIExpression()), !dbg !83
  call fastcc void @_ZN7Vector3IfEC2Efff(%class.Vector3* %0, float %1, float %2, float %3), !dbg !84
  ret void, !dbg !85
}
; CHECK: define void @_ZN7Vector3IfEC1Efff({0: float, 4: float, 8: float}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN7Vector3IfEC2Efff(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN7Vector3IfEC2Efff(%class.Vector3*, float, float, float) unnamed_addr #2 align 2 !dbg !86 {
  call void @llvm.dbg.value(metadata %class.Vector3* %0, metadata !87, metadata !DIExpression()), !dbg !88
  call void @llvm.dbg.value(metadata float %1, metadata !89, metadata !DIExpression()), !dbg !90
  call void @llvm.dbg.value(metadata float %2, metadata !91, metadata !DIExpression()), !dbg !92
  call void @llvm.dbg.value(metadata float %3, metadata !93, metadata !DIExpression()), !dbg !94
  %5 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i64 0, i32 0, !dbg !95
  store float %1, float* %5, align 4, !dbg !95
  %6 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i64 0, i32 1, !dbg !96
  store float %2, float* %6, align 4, !dbg !96
  %7 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i64 0, i32 2, !dbg !97
  store float %3, float* %7, align 4, !dbg !97
  ret void, !dbg !98
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

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !8 {
  %3 = alloca %class.Foo, align 4
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.value(metadata %class.Foo* %3, metadata !19, metadata !DIExpression(DW_OP_deref)), !dbg !42
  call fastcc void @_ZN3FooC1Efff(%class.Foo* nonnull %3, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00), !dbg !42
  call void @llvm.dbg.value(metadata %class.Foo* %3, metadata !19, metadata !DIExpression(DW_OP_deref)), !dbg !42
  %4 = call fastcc { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo* nonnull %3), !dbg !43
  ret i32 0, !dbg !44
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: float, 4: float, 8: float}}* $3 = allocate {0: {0: float, 4: float, 8: float}}, 1, align 4
; CHECK:   call @_ZN3FooC1Efff($3, 1.0E+0, 2.0E+0, 3.0E+0)
; CHECK:   {0: <2 x float>, 8: float} %4 = call @_ZN3Foo9get_coordEv($3)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

attributes #0 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 8.0.0 (tags/RELEASE_800/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "aggregate-in-reg-1.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
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
!43 = !DILocation(line: 21, column: 30, scope: !8)
!44 = !DILocation(line: 22, column: 3, scope: !8)
!45 = distinct !DISubprogram(name: "Foo", linkageName: "_ZN3FooC1Efff", scope: !20, file: !1, line: 15, type: !36, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !35, retainedNodes: !2)
!46 = !DILocalVariable(name: "this", arg: 1, scope: !45, type: !47, flags: DIFlagArtificial | DIFlagObjectPointer)
!47 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!48 = !DILocation(line: 0, scope: !45)
!49 = !DILocalVariable(name: "x", arg: 2, scope: !45, file: !1, line: 15, type: !26)
!50 = !DILocation(line: 15, column: 13, scope: !45)
!51 = !DILocalVariable(name: "y", arg: 3, scope: !45, file: !1, line: 15, type: !26)
!52 = !DILocation(line: 15, column: 22, scope: !45)
!53 = !DILocalVariable(name: "z", arg: 4, scope: !45, file: !1, line: 15, type: !26)
!54 = !DILocation(line: 15, column: 31, scope: !45)
!55 = !DILocation(line: 15, column: 69, scope: !45)
!56 = !DILocation(line: 15, column: 70, scope: !45)
!57 = distinct !DISubprogram(name: "get_coord", linkageName: "_ZN3Foo9get_coordEv", scope: !20, file: !1, line: 16, type: !40, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !39, retainedNodes: !2)
!58 = !DILocalVariable(name: "this", arg: 1, scope: !57, type: !47, flags: DIFlagArtificial | DIFlagObjectPointer)
!59 = !DILocation(line: 0, scope: !57)
!60 = !DILocation(line: 16, column: 41, scope: !57)
!61 = !DILocation(line: 16, column: 34, scope: !57)
!62 = distinct !DISubprogram(name: "Foo", linkageName: "_ZN3FooC2Efff", scope: !20, file: !1, line: 15, type: !36, scopeLine: 15, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !35, retainedNodes: !2)
!63 = !DILocalVariable(name: "this", arg: 1, scope: !62, type: !47, flags: DIFlagArtificial | DIFlagObjectPointer)
!64 = !DILocation(line: 0, scope: !62)
!65 = !DILocalVariable(name: "x", arg: 2, scope: !62, file: !1, line: 15, type: !26)
!66 = !DILocation(line: 15, column: 13, scope: !62)
!67 = !DILocalVariable(name: "y", arg: 3, scope: !62, file: !1, line: 15, type: !26)
!68 = !DILocation(line: 15, column: 22, scope: !62)
!69 = !DILocalVariable(name: "z", arg: 4, scope: !62, file: !1, line: 15, type: !26)
!70 = !DILocation(line: 15, column: 31, scope: !62)
!71 = !DILocation(line: 15, column: 36, scope: !62)
!72 = !DILocation(line: 15, column: 42, scope: !62)
!73 = !DILocation(line: 15, column: 70, scope: !62)
!74 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC1Efff", scope: !23, file: !1, line: 7, type: !30, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !29, retainedNodes: !2)
!75 = !DILocalVariable(name: "this", arg: 1, scope: !74, type: !76, flags: DIFlagArtificial | DIFlagObjectPointer)
!76 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!77 = !DILocation(line: 0, scope: !74)
!78 = !DILocalVariable(name: "x", arg: 2, scope: !74, file: !1, line: 7, type: !26)
!79 = !DILocation(line: 7, column: 18, scope: !74)
!80 = !DILocalVariable(name: "y", arg: 3, scope: !74, file: !1, line: 7, type: !26)
!81 = !DILocation(line: 7, column: 23, scope: !74)
!82 = !DILocalVariable(name: "z", arg: 4, scope: !74, file: !1, line: 7, type: !26)
!83 = !DILocation(line: 7, column: 28, scope: !74)
!84 = !DILocation(line: 7, column: 53, scope: !74)
!85 = !DILocation(line: 7, column: 54, scope: !74)
!86 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC2Efff", scope: !23, file: !1, line: 7, type: !30, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !29, retainedNodes: !2)
!87 = !DILocalVariable(name: "this", arg: 1, scope: !86, type: !76, flags: DIFlagArtificial | DIFlagObjectPointer)
!88 = !DILocation(line: 0, scope: !86)
!89 = !DILocalVariable(name: "x", arg: 2, scope: !86, file: !1, line: 7, type: !26)
!90 = !DILocation(line: 7, column: 18, scope: !86)
!91 = !DILocalVariable(name: "y", arg: 3, scope: !86, file: !1, line: 7, type: !26)
!92 = !DILocation(line: 7, column: 23, scope: !86)
!93 = !DILocalVariable(name: "z", arg: 4, scope: !86, file: !1, line: 7, type: !26)
!94 = !DILocation(line: 7, column: 28, scope: !86)
!95 = !DILocation(line: 7, column: 33, scope: !86)
!96 = !DILocation(line: 7, column: 40, scope: !86)
!97 = !DILocation(line: 7, column: 47, scope: !86)
!98 = !DILocation(line: 7, column: 54, scope: !86)
