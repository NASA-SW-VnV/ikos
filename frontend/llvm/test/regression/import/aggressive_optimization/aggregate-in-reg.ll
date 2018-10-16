; ModuleID = 'aggregate-in-reg.cpp.pp.bc'
source_filename = "aggregate-in-reg.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

%class.Foo = type { %class.Vector3 }
%class.Vector3 = type { float, float, float }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo*) unnamed_addr #3 align 2 !dbg !60 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, i64 0, metadata !61, metadata !15), !dbg !62
  %.sroa.02.0..sroa_cast = bitcast %class.Foo* %0 to <2 x float>*, !dbg !63
  %.sroa.02.0.copyload = load <2 x float>, <2 x float>* %.sroa.02.0..sroa_cast, align 4, !dbg !63
  %.sroa.23.0..sroa_idx4 = getelementptr inbounds %class.Foo, %class.Foo* %0, i64 0, i32 0, i32 2, !dbg !63
  %.sroa.23.0.copyload = load float, float* %.sroa.23.0..sroa_idx4, align 4, !dbg !63
  %.fca.0.insert = insertvalue { <2 x float>, float } undef, <2 x float> %.sroa.02.0.copyload, 0, !dbg !64
  %.fca.1.insert = insertvalue { <2 x float>, float } %.fca.0.insert, float %.sroa.23.0.copyload, 1, !dbg !64
  ret { <2 x float>, float } %.fca.1.insert, !dbg !64
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
define internal fastcc void @_ZN3FooC1Efff(%class.Foo*, float, float, float) unnamed_addr #2 align 2 !dbg !48 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, i64 0, metadata !49, metadata !15), !dbg !51
  call void @llvm.dbg.value(metadata float %1, i64 0, metadata !52, metadata !15), !dbg !53
  call void @llvm.dbg.value(metadata float %2, i64 0, metadata !54, metadata !15), !dbg !55
  call void @llvm.dbg.value(metadata float %3, i64 0, metadata !56, metadata !15), !dbg !57
  call fastcc void @_ZN3FooC2Efff(%class.Foo* %0, float %1, float %2, float %3), !dbg !58
  ret void, !dbg !59
}
; CHECK: define void @_ZN3FooC1Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN3FooC2Efff(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define internal fastcc void @_ZN3FooC2Efff(%class.Foo*, float, float, float) unnamed_addr #2 align 2 !dbg !65 {
  call void @llvm.dbg.value(metadata %class.Foo* %0, i64 0, metadata !66, metadata !15), !dbg !67
  call void @llvm.dbg.value(metadata float %1, i64 0, metadata !68, metadata !15), !dbg !69
  call void @llvm.dbg.value(metadata float %2, i64 0, metadata !70, metadata !15), !dbg !71
  call void @llvm.dbg.value(metadata float %3, i64 0, metadata !72, metadata !15), !dbg !73
  %5 = getelementptr inbounds %class.Foo, %class.Foo* %0, i64 0, i32 0, !dbg !74
  call fastcc void @_ZN7Vector3IfEC1Efff(%class.Vector3* %5, float %1, float %2, float %3), !dbg !75
  ret void, !dbg !76
}
; CHECK: define void @_ZN3FooC2Efff({0: {0: float, 4: float, 8: float}}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: float, 4: float, 8: float}* %5 = ptrshift %1, 12 * 0, 1 * 0
; CHECK:   call @_ZN7Vector3IfEC1Efff(%5, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define internal fastcc void @_ZN7Vector3IfEC1Efff(%class.Vector3*, float, float, float) unnamed_addr #2 align 2 !dbg !77 {
  call void @llvm.dbg.value(metadata %class.Vector3* %0, i64 0, metadata !78, metadata !15), !dbg !80
  call void @llvm.dbg.value(metadata float %1, i64 0, metadata !81, metadata !15), !dbg !82
  call void @llvm.dbg.value(metadata float %2, i64 0, metadata !83, metadata !15), !dbg !84
  call void @llvm.dbg.value(metadata float %3, i64 0, metadata !85, metadata !15), !dbg !86
  call fastcc void @_ZN7Vector3IfEC2Efff(%class.Vector3* %0, float %1, float %2, float %3), !dbg !87
  ret void, !dbg !88
}
; CHECK: define void @_ZN7Vector3IfEC1Efff({0: float, 4: float, 8: float}* %1, float %2, float %3, float %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN7Vector3IfEC2Efff(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN7Vector3IfEC2Efff(%class.Vector3*, float, float, float) unnamed_addr #3 align 2 !dbg !89 {
  call void @llvm.dbg.value(metadata %class.Vector3* %0, i64 0, metadata !90, metadata !15), !dbg !91
  call void @llvm.dbg.value(metadata float %1, i64 0, metadata !92, metadata !15), !dbg !93
  call void @llvm.dbg.value(metadata float %2, i64 0, metadata !94, metadata !15), !dbg !95
  call void @llvm.dbg.value(metadata float %3, i64 0, metadata !96, metadata !15), !dbg !97
  %5 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i64 0, i32 0, !dbg !98
  store float %1, float* %5, align 4, !dbg !98
  %6 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i64 0, i32 1, !dbg !99
  store float %2, float* %6, align 4, !dbg !99
  %7 = getelementptr inbounds %class.Vector3, %class.Vector3* %0, i64 0, i32 2, !dbg !100
  store float %3, float* %7, align 4, !dbg !100
  ret void, !dbg !101
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
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !7 {
  %3 = alloca %class.Foo, align 4
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !14, metadata !15), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !17, metadata !15), !dbg !18
  call void @llvm.dbg.value(metadata %class.Foo* %3, i64 0, metadata !19, metadata !42), !dbg !43
  call fastcc void @_ZN3FooC1Efff(%class.Foo* nonnull %3, float 1.000000e+00, float 2.000000e+00, float 3.000000e+00), !dbg !43
  call void @llvm.dbg.declare(metadata %class.Vector3* undef, metadata !44, metadata !15), !dbg !45
  call void @llvm.dbg.value(metadata %class.Foo* %3, i64 0, metadata !19, metadata !42), !dbg !43
  %4 = call fastcc { <2 x float>, float } @_ZN3Foo9get_coordEv(%class.Foo* nonnull %3), !dbg !46
  ret i32 0, !dbg !47
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: float, 4: float, 8: float}}* $3 = allocate {0: {0: float, 4: float, 8: float}}, 1, align 4
; CHECK:   call @_ZN3FooC1Efff($3, 1.0E+0, 2.0E+0, 3.0E+0)
; CHECK:   {0: <2 x float>, 8: float} %4 = call @_ZN3Foo9get_coordEv($3)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "aggregate-in-reg.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 19, type: !8, isLocal: false, isDefinition: true, scopeLine: 19, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!14 = !DILocalVariable(name: "argc", arg: 1, scope: !7, file: !1, line: 19, type: !10)
!15 = !DIExpression()
!16 = !DILocation(line: 19, column: 14, scope: !7)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !7, file: !1, line: 19, type: !11)
!18 = !DILocation(line: 19, column: 26, scope: !7)
!19 = !DILocalVariable(name: "f", scope: !7, file: !1, line: 20, type: !20)
!20 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Foo", file: !1, line: 10, size: 96, elements: !21, identifier: "_ZTS3Foo")
!21 = !{!22, !35, !39}
!22 = !DIDerivedType(tag: DW_TAG_member, name: "coord", scope: !20, file: !1, line: 12, baseType: !23, size: 96)
!23 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Vector3<float>", file: !1, line: 2, size: 96, elements: !24, templateParams: !33, identifier: "_ZTS7Vector3IfE")
!24 = !{!25, !27, !28, !29}
!25 = !DIDerivedType(tag: DW_TAG_member, name: "_x", scope: !23, file: !1, line: 4, baseType: !26, size: 32)
!26 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "_y", scope: !23, file: !1, line: 4, baseType: !26, size: 32, offset: 32)
!28 = !DIDerivedType(tag: DW_TAG_member, name: "_z", scope: !23, file: !1, line: 4, baseType: !26, size: 32, offset: 64)
!29 = !DISubprogram(name: "Vector3", scope: !23, file: !1, line: 7, type: !30, isLocal: false, isDefinition: false, scopeLine: 7, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!30 = !DISubroutineType(types: !31)
!31 = !{null, !32, !26, !26, !26}
!32 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!33 = !{!34}
!34 = !DITemplateTypeParameter(name: "T", type: !26)
!35 = !DISubprogram(name: "Foo", scope: !20, file: !1, line: 15, type: !36, isLocal: false, isDefinition: false, scopeLine: 15, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!36 = !DISubroutineType(types: !37)
!37 = !{null, !38, !26, !26, !26}
!38 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!39 = !DISubprogram(name: "get_coord", linkageName: "_ZN3Foo9get_coordEv", scope: !20, file: !1, line: 16, type: !40, isLocal: false, isDefinition: false, scopeLine: 16, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!40 = !DISubroutineType(types: !41)
!41 = !{!23, !38}
!42 = !DIExpression(DW_OP_deref)
!43 = !DILocation(line: 20, column: 7, scope: !7)
!44 = !DILocalVariable(name: "coord", scope: !7, file: !1, line: 21, type: !23)
!45 = !DILocation(line: 21, column: 20, scope: !7)
!46 = !DILocation(line: 21, column: 30, scope: !7)
!47 = !DILocation(line: 22, column: 3, scope: !7)
!48 = distinct !DISubprogram(name: "Foo", linkageName: "_ZN3FooC1Efff", scope: !20, file: !1, line: 15, type: !36, isLocal: false, isDefinition: true, scopeLine: 15, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !35, variables: !2)
!49 = !DILocalVariable(name: "this", arg: 1, scope: !48, type: !50, flags: DIFlagArtificial | DIFlagObjectPointer)
!50 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!51 = !DILocation(line: 0, scope: !48)
!52 = !DILocalVariable(name: "x", arg: 2, scope: !48, file: !1, line: 15, type: !26)
!53 = !DILocation(line: 15, column: 13, scope: !48)
!54 = !DILocalVariable(name: "y", arg: 3, scope: !48, file: !1, line: 15, type: !26)
!55 = !DILocation(line: 15, column: 22, scope: !48)
!56 = !DILocalVariable(name: "z", arg: 4, scope: !48, file: !1, line: 15, type: !26)
!57 = !DILocation(line: 15, column: 31, scope: !48)
!58 = !DILocation(line: 15, column: 69, scope: !48)
!59 = !DILocation(line: 15, column: 70, scope: !48)
!60 = distinct !DISubprogram(name: "get_coord", linkageName: "_ZN3Foo9get_coordEv", scope: !20, file: !1, line: 16, type: !40, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !39, variables: !2)
!61 = !DILocalVariable(name: "this", arg: 1, scope: !60, type: !50, flags: DIFlagArtificial | DIFlagObjectPointer)
!62 = !DILocation(line: 0, scope: !60)
!63 = !DILocation(line: 16, column: 41, scope: !60)
!64 = !DILocation(line: 16, column: 34, scope: !60)
!65 = distinct !DISubprogram(name: "Foo", linkageName: "_ZN3FooC2Efff", scope: !20, file: !1, line: 15, type: !36, isLocal: false, isDefinition: true, scopeLine: 15, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !35, variables: !2)
!66 = !DILocalVariable(name: "this", arg: 1, scope: !65, type: !50, flags: DIFlagArtificial | DIFlagObjectPointer)
!67 = !DILocation(line: 0, scope: !65)
!68 = !DILocalVariable(name: "x", arg: 2, scope: !65, file: !1, line: 15, type: !26)
!69 = !DILocation(line: 15, column: 13, scope: !65)
!70 = !DILocalVariable(name: "y", arg: 3, scope: !65, file: !1, line: 15, type: !26)
!71 = !DILocation(line: 15, column: 22, scope: !65)
!72 = !DILocalVariable(name: "z", arg: 4, scope: !65, file: !1, line: 15, type: !26)
!73 = !DILocation(line: 15, column: 31, scope: !65)
!74 = !DILocation(line: 15, column: 36, scope: !65)
!75 = !DILocation(line: 15, column: 42, scope: !65)
!76 = !DILocation(line: 15, column: 70, scope: !65)
!77 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC1Efff", scope: !23, file: !1, line: 7, type: !30, isLocal: false, isDefinition: true, scopeLine: 7, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !29, variables: !2)
!78 = !DILocalVariable(name: "this", arg: 1, scope: !77, type: !79, flags: DIFlagArtificial | DIFlagObjectPointer)
!79 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!80 = !DILocation(line: 0, scope: !77)
!81 = !DILocalVariable(name: "x", arg: 2, scope: !77, file: !1, line: 7, type: !26)
!82 = !DILocation(line: 7, column: 18, scope: !77)
!83 = !DILocalVariable(name: "y", arg: 3, scope: !77, file: !1, line: 7, type: !26)
!84 = !DILocation(line: 7, column: 23, scope: !77)
!85 = !DILocalVariable(name: "z", arg: 4, scope: !77, file: !1, line: 7, type: !26)
!86 = !DILocation(line: 7, column: 28, scope: !77)
!87 = !DILocation(line: 7, column: 53, scope: !77)
!88 = !DILocation(line: 7, column: 54, scope: !77)
!89 = distinct !DISubprogram(name: "Vector3", linkageName: "_ZN7Vector3IfEC2Efff", scope: !23, file: !1, line: 7, type: !30, isLocal: false, isDefinition: true, scopeLine: 7, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !29, variables: !2)
!90 = !DILocalVariable(name: "this", arg: 1, scope: !89, type: !79, flags: DIFlagArtificial | DIFlagObjectPointer)
!91 = !DILocation(line: 0, scope: !89)
!92 = !DILocalVariable(name: "x", arg: 2, scope: !89, file: !1, line: 7, type: !26)
!93 = !DILocation(line: 7, column: 18, scope: !89)
!94 = !DILocalVariable(name: "y", arg: 3, scope: !89, file: !1, line: 7, type: !26)
!95 = !DILocation(line: 7, column: 23, scope: !89)
!96 = !DILocalVariable(name: "z", arg: 4, scope: !89, file: !1, line: 7, type: !26)
!97 = !DILocation(line: 7, column: 28, scope: !89)
!98 = !DILocation(line: 7, column: 33, scope: !89)
!99 = !DILocation(line: 7, column: 40, scope: !89)
!100 = !DILocation(line: 7, column: 47, scope: !89)
!101 = !DILocation(line: 7, column: 54, scope: !89)
