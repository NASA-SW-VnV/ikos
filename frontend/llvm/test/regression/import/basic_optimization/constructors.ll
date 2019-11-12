; ModuleID = 'constructors.pp.bc'
source_filename = "constructors.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%class.Vector = type { i32, i32, i32 }
%class.Master = type { %class.Vector*, i32* }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @_Z1fP6Vector(%class.Vector*) #0 !dbg !8 {
  call void @llvm.dbg.value(metadata %class.Vector* %0, metadata !22, metadata !DIExpression()), !dbg !23
  %2 = getelementptr inbounds %class.Vector, %class.Vector* %0, i32 0, i32 1, !dbg !24
  %3 = load i32, i32* %2, align 4, !dbg !24
  ret i32 %3, !dbg !25
}
; CHECK: define si32 @_Z1fP6Vector({0: si32, 4: si32, 8: si32}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32* %2 = ptrshift %1, 12 * 0, 1 * 4
; CHECK:   si32 %3 = load %2, align 4
; CHECK:   return %3
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN6MasterC1Ev(%class.Master*) unnamed_addr #3 align 2 !dbg !41 {
  call void @llvm.dbg.value(metadata %class.Master* %0, metadata !42, metadata !DIExpression()), !dbg !44
  call void @_ZN6MasterC2Ev(%class.Master* %0), !dbg !45
  ret void, !dbg !46
}
; CHECK: define void @_ZN6MasterC1Ev({0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN6MasterC2Ev(%1)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN6MasterC2Ev(%class.Master*) unnamed_addr #3 align 2 !dbg !47 {
  call void @llvm.dbg.value(metadata %class.Master* %0, metadata !48, metadata !DIExpression()), !dbg !49
  %2 = call i8* @_Znwm(i64 12) #6, !dbg !50
  %3 = bitcast i8* %2 to %class.Vector*, !dbg !50
  call void @_ZN6VectorC1Eiii(%class.Vector* %3, i32 1, i32 2, i32 3) #7, !dbg !52
  %4 = getelementptr inbounds %class.Master, %class.Master* %0, i32 0, i32 0, !dbg !53
  store %class.Vector* %3, %class.Vector** %4, align 8, !dbg !54
  %5 = call i8* @_Znwm(i64 4) #6, !dbg !55
  %6 = bitcast i8* %5 to i32*, !dbg !55
  store i32 4, i32* %6, align 4, !dbg !55
  %7 = getelementptr inbounds %class.Master, %class.Master* %0, i32 0, i32 1, !dbg !56
  store i32* %6, i32** %7, align 8, !dbg !57
  %8 = getelementptr inbounds %class.Master, %class.Master* %0, i32 0, i32 0, !dbg !58
  %9 = load %class.Vector*, %class.Vector** %8, align 8, !dbg !58
  %10 = call i32 @_Z1fP6Vector(%class.Vector* %9), !dbg !59
  %11 = icmp eq i32 %10, 2, !dbg !60
  %12 = zext i1 %11 to i32, !dbg !59
  call void @__ikos_assert(i32 %12), !dbg !61
  ret void, !dbg !62
}
; CHECK: define void @_ZN6MasterC2Ev({0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* %1) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si8* %2 = call @ar.libcpp.new(12)
; CHECK:   {0: si32, 4: si32, 8: si32}* %3 = bitcast %2
; CHECK:   call @_ZN6VectorC1Eiii(%3, 1, 2, 3)
; CHECK:   {0: si32, 4: si32, 8: si32}** %4 = ptrshift %1, 16 * 0, 1 * 0
; CHECK:   store %4, %3, align 8
; CHECK:   si8* %5 = call @ar.libcpp.new(4)
; CHECK:   si32* %6 = bitcast %5
; CHECK:   store %6, 4, align 4
; CHECK:   si32** %7 = ptrshift %1, 16 * 0, 1 * 8
; CHECK:   store %7, %6, align 8
; CHECK:   {0: si32, 4: si32, 8: si32}** %8 = ptrshift %1, 16 * 0, 1 * 0
; CHECK:   {0: si32, 4: si32, 8: si32}** %9 = bitcast %8
; CHECK:   {0: si32, 4: si32, 8: si32}* %10 = load %9, align 8
; CHECK:   si32 %11 = call @_Z1fP6Vector(%10)
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   %11 sieq 2
; CHECK:   ui1 %12 = 1
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   %11 sine 2
; CHECK:   ui1 %12 = 0
; CHECK: }
; CHECK: #4 !exit predecessors={#2, #3} {
; CHECK:   ui32 %13 = zext %12
; CHECK:   call @ar.ikos.assert(%13)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN6VectorC1Eiii(%class.Vector*, i32, i32, i32) unnamed_addr #0 align 2 !dbg !63 {
  call void @llvm.dbg.value(metadata %class.Vector* %0, metadata !64, metadata !DIExpression()), !dbg !65
  call void @llvm.dbg.value(metadata i32 %1, metadata !66, metadata !DIExpression()), !dbg !65
  call void @llvm.dbg.value(metadata i32 %2, metadata !67, metadata !DIExpression()), !dbg !65
  call void @llvm.dbg.value(metadata i32 %3, metadata !68, metadata !DIExpression()), !dbg !65
  call void @_ZN6VectorC2Eiii(%class.Vector* %0, i32 %1, i32 %2, i32 %3) #7, !dbg !69
  ret void, !dbg !70
}
; CHECK: define void @_ZN6VectorC1Eiii({0: si32, 4: si32, 8: si32}* %1, si32 %2, si32 %3, si32 %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN6VectorC2Eiii(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN6VectorC2Eiii(%class.Vector*, i32, i32, i32) unnamed_addr #0 align 2 !dbg !71 {
  call void @llvm.dbg.value(metadata %class.Vector* %0, metadata !72, metadata !DIExpression()), !dbg !73
  call void @llvm.dbg.value(metadata i32 %1, metadata !74, metadata !DIExpression()), !dbg !73
  call void @llvm.dbg.value(metadata i32 %2, metadata !75, metadata !DIExpression()), !dbg !73
  call void @llvm.dbg.value(metadata i32 %3, metadata !76, metadata !DIExpression()), !dbg !73
  %5 = getelementptr inbounds %class.Vector, %class.Vector* %0, i32 0, i32 0, !dbg !77
  store i32 %1, i32* %5, align 4, !dbg !77
  %6 = getelementptr inbounds %class.Vector, %class.Vector* %0, i32 0, i32 1, !dbg !78
  store i32 %2, i32* %6, align 4, !dbg !78
  %7 = getelementptr inbounds %class.Vector, %class.Vector* %0, i32 0, i32 2, !dbg !79
  store i32 %3, i32* %7, align 4, !dbg !79
  ret void, !dbg !80
}
; CHECK: define void @_ZN6VectorC2Eiii({0: si32, 4: si32, 8: si32}* %1, si32 %2, si32 %3, si32 %4) {
; CHECK: #1 !entry !exit {
; CHECK:   si32* %5 = ptrshift %1, 12 * 0, 1 * 0
; CHECK:   store %5, %2, align 4
; CHECK:   si32* %6 = ptrshift %1, 12 * 0, 1 * 4
; CHECK:   store %6, %3, align 4
; CHECK:   si32* %7 = ptrshift %1, 12 * 0, 1 * 8
; CHECK:   store %7, %4, align 4
; CHECK:   return
; CHECK: }
; CHECK: }

declare void @__ikos_assert(i32) #5
; CHECK: declare void @ar.ikos.assert(ui32)

; Function Attrs: nobuiltin
declare noalias i8* @_Znwm(i64) #4
; CHECK: declare si8* @ar.libcpp.new(ui64)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main() #2 !dbg !26 {
  %1 = alloca %class.Master, align 8
  call void @llvm.dbg.declare(metadata %class.Master* %1, metadata !29, metadata !DIExpression()), !dbg !39
  call void @_ZN6MasterC1Ev(%class.Master* %1), !dbg !39
  ret i32 0, !dbg !40
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* $1 = allocate {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}, 1, align 8
; CHECK:   call @_ZN6MasterC1Ev($1)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { builtin }
attributes #7 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "constructors.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "f", linkageName: "_Z1fP6Vector", scope: !1, file: !1, line: 14, type: !9, scopeLine: 14, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Vector", file: !1, line: 5, size: 96, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !14, identifier: "_ZTS6Vector")
!14 = !{!15, !16, !17, !18}
!15 = !DIDerivedType(tag: DW_TAG_member, name: "_x", scope: !13, file: !1, line: 7, baseType: !11, size: 32, flags: DIFlagPublic)
!16 = !DIDerivedType(tag: DW_TAG_member, name: "_y", scope: !13, file: !1, line: 8, baseType: !11, size: 32, offset: 32, flags: DIFlagPublic)
!17 = !DIDerivedType(tag: DW_TAG_member, name: "_z", scope: !13, file: !1, line: 9, baseType: !11, size: 32, offset: 64, flags: DIFlagPublic)
!18 = !DISubprogram(name: "Vector", scope: !13, file: !1, line: 11, type: !19, scopeLine: 11, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!19 = !DISubroutineType(types: !20)
!20 = !{null, !21, !11, !11, !11}
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!22 = !DILocalVariable(name: "v", arg: 1, scope: !8, file: !1, line: 14, type: !12)
!23 = !DILocation(line: 0, scope: !8)
!24 = !DILocation(line: 15, column: 13, scope: !8)
!25 = !DILocation(line: 15, column: 3, scope: !8)
!26 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 30, type: !27, scopeLine: 30, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!27 = !DISubroutineType(types: !28)
!28 = !{!11}
!29 = !DILocalVariable(name: "master", scope: !26, file: !1, line: 31, type: !30)
!30 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Master", file: !1, line: 18, size: 128, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !31, identifier: "_ZTS6Master")
!31 = !{!32, !33, !35}
!32 = !DIDerivedType(tag: DW_TAG_member, name: "_v", scope: !30, file: !1, line: 20, baseType: !12, size: 64, flags: DIFlagPublic)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "_p", scope: !30, file: !1, line: 21, baseType: !34, size: 64, offset: 64, flags: DIFlagPublic)
!34 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!35 = !DISubprogram(name: "Master", scope: !30, file: !1, line: 23, type: !36, scopeLine: 23, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!36 = !DISubroutineType(types: !37)
!37 = !{null, !38}
!38 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !30, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!39 = !DILocation(line: 31, column: 10, scope: !26)
!40 = !DILocation(line: 32, column: 3, scope: !26)
!41 = distinct !DISubprogram(name: "Master", linkageName: "_ZN6MasterC1Ev", scope: !30, file: !1, line: 23, type: !36, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !35, retainedNodes: !2)
!42 = !DILocalVariable(name: "this", arg: 1, scope: !41, type: !43, flags: DIFlagArtificial | DIFlagObjectPointer)
!43 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !30, size: 64)
!44 = !DILocation(line: 0, scope: !41)
!45 = !DILocation(line: 23, column: 12, scope: !41)
!46 = !DILocation(line: 27, column: 3, scope: !41)
!47 = distinct !DISubprogram(name: "Master", linkageName: "_ZN6MasterC2Ev", scope: !30, file: !1, line: 23, type: !36, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !35, retainedNodes: !2)
!48 = !DILocalVariable(name: "this", arg: 1, scope: !47, type: !43, flags: DIFlagArtificial | DIFlagObjectPointer)
!49 = !DILocation(line: 0, scope: !47)
!50 = !DILocation(line: 24, column: 10, scope: !51)
!51 = distinct !DILexicalBlock(scope: !47, file: !1, line: 23, column: 12)
!52 = !DILocation(line: 24, column: 14, scope: !51)
!53 = !DILocation(line: 24, column: 5, scope: !51)
!54 = !DILocation(line: 24, column: 8, scope: !51)
!55 = !DILocation(line: 25, column: 10, scope: !51)
!56 = !DILocation(line: 25, column: 5, scope: !51)
!57 = !DILocation(line: 25, column: 8, scope: !51)
!58 = !DILocation(line: 26, column: 21, scope: !51)
!59 = !DILocation(line: 26, column: 19, scope: !51)
!60 = !DILocation(line: 26, column: 25, scope: !51)
!61 = !DILocation(line: 26, column: 5, scope: !51)
!62 = !DILocation(line: 27, column: 3, scope: !47)
!63 = distinct !DISubprogram(name: "Vector", linkageName: "_ZN6VectorC1Eiii", scope: !13, file: !1, line: 11, type: !19, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !18, retainedNodes: !2)
!64 = !DILocalVariable(name: "this", arg: 1, scope: !63, type: !12, flags: DIFlagArtificial | DIFlagObjectPointer)
!65 = !DILocation(line: 0, scope: !63)
!66 = !DILocalVariable(name: "x", arg: 2, scope: !63, file: !1, line: 11, type: !11)
!67 = !DILocalVariable(name: "y", arg: 3, scope: !63, file: !1, line: 11, type: !11)
!68 = !DILocalVariable(name: "z", arg: 4, scope: !63, file: !1, line: 11, type: !11)
!69 = !DILocation(line: 11, column: 62, scope: !63)
!70 = !DILocation(line: 11, column: 63, scope: !63)
!71 = distinct !DISubprogram(name: "Vector", linkageName: "_ZN6VectorC2Eiii", scope: !13, file: !1, line: 11, type: !19, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !18, retainedNodes: !2)
!72 = !DILocalVariable(name: "this", arg: 1, scope: !71, type: !12, flags: DIFlagArtificial | DIFlagObjectPointer)
!73 = !DILocation(line: 0, scope: !71)
!74 = !DILocalVariable(name: "x", arg: 2, scope: !71, file: !1, line: 11, type: !11)
!75 = !DILocalVariable(name: "y", arg: 3, scope: !71, file: !1, line: 11, type: !11)
!76 = !DILocalVariable(name: "z", arg: 4, scope: !71, file: !1, line: 11, type: !11)
!77 = !DILocation(line: 11, column: 42, scope: !71)
!78 = !DILocation(line: 11, column: 49, scope: !71)
!79 = !DILocation(line: 11, column: 56, scope: !71)
!80 = !DILocation(line: 11, column: 63, scope: !71)
