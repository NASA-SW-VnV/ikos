; ModuleID = 'constructors.cpp.pp.bc'
source_filename = "constructors.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

%class.Vector = type { i32, i32, i32 }
%class.Master = type { %class.Vector*, i32* }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc i32 @_Z1fP6Vector(%class.Vector*) unnamed_addr #0 !dbg !7 {
  call void @llvm.dbg.value(metadata %class.Vector* %0, i64 0, metadata !21, metadata !22), !dbg !23
  %2 = getelementptr inbounds %class.Vector, %class.Vector* %0, i64 0, i32 1, !dbg !24
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
define internal fastcc void @_ZN6MasterC1Ev(%class.Master*) unnamed_addr #2 align 2 !dbg !42 {
  call void @llvm.dbg.value(metadata %class.Master* %0, i64 0, metadata !43, metadata !22), !dbg !45
  call fastcc void @_ZN6MasterC2Ev(%class.Master* %0), !dbg !46
  ret void, !dbg !47
}
; CHECK: define void @_ZN6MasterC1Ev({0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN6MasterC2Ev(%1)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define internal fastcc void @_ZN6MasterC2Ev(%class.Master*) unnamed_addr #2 align 2 !dbg !48 {
  call void @llvm.dbg.value(metadata %class.Master* %0, i64 0, metadata !49, metadata !22), !dbg !50
  %2 = call i8* @_Znwm(i64 12) #6, !dbg !51
  %3 = bitcast i8* %2 to %class.Vector*, !dbg !51
  call fastcc void @_ZN6VectorC1Eiii(%class.Vector* %3, i32 1, i32 2, i32 3) #7, !dbg !53
  %4 = bitcast %class.Master* %0 to i8**, !dbg !55
  store i8* %2, i8** %4, align 8, !dbg !55
  %5 = call i8* @_Znwm(i64 4) #6, !dbg !56
  %6 = bitcast i8* %5 to i32*, !dbg !56
  store i32 4, i32* %6, align 4, !dbg !56
  %7 = getelementptr inbounds %class.Master, %class.Master* %0, i64 0, i32 1, !dbg !57
  %8 = bitcast i32** %7 to i8**, !dbg !58
  store i8* %5, i8** %8, align 8, !dbg !58
  %9 = call fastcc i32 @_Z1fP6Vector(%class.Vector* %3), !dbg !59
  %10 = icmp eq i32 %9, 2, !dbg !60
  %11 = zext i1 %10 to i32, !dbg !59
  call void @__ikos_assert(i32 %11), !dbg !61
  ret void, !dbg !62
}
; CHECK: define void @_ZN6MasterC2Ev({0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* %1) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si8* %2 = call @ar.libcpp.new(12)
; CHECK:   {0: si32, 4: si32, 8: si32}* %3 = bitcast %2
; CHECK:   call @_ZN6VectorC1Eiii(%3, 1, 2, 3)
; CHECK:   si8** %4 = bitcast %1
; CHECK:   store %4, %2, align 8
; CHECK:   si8* %5 = call @ar.libcpp.new(4)
; CHECK:   si32* %6 = bitcast %5
; CHECK:   store %6, 4, align 4
; CHECK:   si32** %7 = ptrshift %1, 16 * 0, 1 * 8
; CHECK:   si8** %8 = bitcast %7
; CHECK:   store %8, %5, align 8
; CHECK:   si32 %9 = call @_Z1fP6Vector(%3)
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   %9 sieq 2
; CHECK:   ui1 %10 = 1
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   %9 sine 2
; CHECK:   ui1 %10 = 0
; CHECK: }
; CHECK: #4 !exit predecessors={#2, #3} {
; CHECK:   ui32 %11 = zext %10
; CHECK:   call @ar.ikos.assert(%11)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN6VectorC1Eiii(%class.Vector*, i32, i32, i32) unnamed_addr #0 align 2 !dbg !63 {
  call void @llvm.dbg.value(metadata %class.Vector* %0, i64 0, metadata !64, metadata !22), !dbg !65
  call void @llvm.dbg.value(metadata i32 %1, i64 0, metadata !66, metadata !22), !dbg !67
  call void @llvm.dbg.value(metadata i32 %2, i64 0, metadata !68, metadata !22), !dbg !69
  call void @llvm.dbg.value(metadata i32 %3, i64 0, metadata !70, metadata !22), !dbg !71
  call fastcc void @_ZN6VectorC2Eiii(%class.Vector* %0, i32 %1, i32 %2, i32 %3) #7, !dbg !72
  ret void, !dbg !73
}
; CHECK: define void @_ZN6VectorC1Eiii({0: si32, 4: si32, 8: si32}* %1, si32 %2, si32 %3, si32 %4) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN6VectorC2Eiii(%1, %2, %3, %4)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN6VectorC2Eiii(%class.Vector*, i32, i32, i32) unnamed_addr #0 align 2 !dbg !74 {
  call void @llvm.dbg.value(metadata %class.Vector* %0, i64 0, metadata !75, metadata !22), !dbg !76
  call void @llvm.dbg.value(metadata i32 %1, i64 0, metadata !77, metadata !22), !dbg !78
  call void @llvm.dbg.value(metadata i32 %2, i64 0, metadata !79, metadata !22), !dbg !80
  call void @llvm.dbg.value(metadata i32 %3, i64 0, metadata !81, metadata !22), !dbg !82
  %5 = getelementptr inbounds %class.Vector, %class.Vector* %0, i64 0, i32 0, !dbg !83
  store i32 %1, i32* %5, align 4, !dbg !83
  %6 = getelementptr inbounds %class.Vector, %class.Vector* %0, i64 0, i32 1, !dbg !84
  store i32 %2, i32* %6, align 4, !dbg !84
  %7 = getelementptr inbounds %class.Vector, %class.Vector* %0, i64 0, i32 2, !dbg !85
  store i32 %3, i32* %7, align 4, !dbg !85
  ret void, !dbg !86
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

declare void @__ikos_assert(i32) local_unnamed_addr #4
; CHECK: declare void @ar.ikos.assert(ui32)

; Function Attrs: nobuiltin
declare noalias i8* @_Znwm(i64) local_unnamed_addr #3
; CHECK: declare si8* @ar.libcpp.new(ui64)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main() local_unnamed_addr #1 !dbg !26 {
  %1 = alloca %class.Master, align 8
  call void @llvm.dbg.value(metadata %class.Master* %1, i64 0, metadata !29, metadata !39), !dbg !40
  call fastcc void @_ZN6MasterC1Ev(%class.Master* nonnull %1), !dbg !40
  ret i32 0, !dbg !41
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* $1 = allocate {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}, 1, align 8
; CHECK:   call @_ZN6MasterC1Ev($1)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #5

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nobuiltin "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind readnone }
attributes #6 = { builtin }
attributes #7 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "constructors.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "f", linkageName: "_Z1fP6Vector", scope: !1, file: !1, line: 14, type: !8, isLocal: false, isDefinition: true, scopeLine: 14, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Vector", file: !1, line: 5, size: 96, elements: !13, identifier: "_ZTS6Vector")
!13 = !{!14, !15, !16, !17}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "_x", scope: !12, file: !1, line: 7, baseType: !10, size: 32, flags: DIFlagPublic)
!15 = !DIDerivedType(tag: DW_TAG_member, name: "_y", scope: !12, file: !1, line: 8, baseType: !10, size: 32, offset: 32, flags: DIFlagPublic)
!16 = !DIDerivedType(tag: DW_TAG_member, name: "_z", scope: !12, file: !1, line: 9, baseType: !10, size: 32, offset: 64, flags: DIFlagPublic)
!17 = !DISubprogram(name: "Vector", scope: !12, file: !1, line: 11, type: !18, isLocal: false, isDefinition: false, scopeLine: 11, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!18 = !DISubroutineType(types: !19)
!19 = !{null, !20, !10, !10, !10}
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!21 = !DILocalVariable(name: "v", arg: 1, scope: !7, file: !1, line: 14, type: !11)
!22 = !DIExpression()
!23 = !DILocation(line: 14, column: 15, scope: !7)
!24 = !DILocation(line: 15, column: 13, scope: !7)
!25 = !DILocation(line: 15, column: 3, scope: !7)
!26 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 30, type: !27, isLocal: false, isDefinition: true, scopeLine: 30, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!27 = !DISubroutineType(types: !28)
!28 = !{!10}
!29 = !DILocalVariable(name: "master", scope: !26, file: !1, line: 31, type: !30)
!30 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Master", file: !1, line: 18, size: 128, elements: !31, identifier: "_ZTS6Master")
!31 = !{!32, !33, !35}
!32 = !DIDerivedType(tag: DW_TAG_member, name: "_v", scope: !30, file: !1, line: 20, baseType: !11, size: 64, flags: DIFlagPublic)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "_p", scope: !30, file: !1, line: 21, baseType: !34, size: 64, offset: 64, flags: DIFlagPublic)
!34 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!35 = !DISubprogram(name: "Master", scope: !30, file: !1, line: 23, type: !36, isLocal: false, isDefinition: false, scopeLine: 23, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!36 = !DISubroutineType(types: !37)
!37 = !{null, !38}
!38 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !30, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!39 = !DIExpression(DW_OP_deref)
!40 = !DILocation(line: 31, column: 10, scope: !26)
!41 = !DILocation(line: 32, column: 3, scope: !26)
!42 = distinct !DISubprogram(name: "Master", linkageName: "_ZN6MasterC1Ev", scope: !30, file: !1, line: 23, type: !36, isLocal: false, isDefinition: true, scopeLine: 23, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !35, variables: !2)
!43 = !DILocalVariable(name: "this", arg: 1, scope: !42, type: !44, flags: DIFlagArtificial | DIFlagObjectPointer)
!44 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !30, size: 64)
!45 = !DILocation(line: 0, scope: !42)
!46 = !DILocation(line: 23, column: 12, scope: !42)
!47 = !DILocation(line: 27, column: 3, scope: !42)
!48 = distinct !DISubprogram(name: "Master", linkageName: "_ZN6MasterC2Ev", scope: !30, file: !1, line: 23, type: !36, isLocal: false, isDefinition: true, scopeLine: 23, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !35, variables: !2)
!49 = !DILocalVariable(name: "this", arg: 1, scope: !48, type: !44, flags: DIFlagArtificial | DIFlagObjectPointer)
!50 = !DILocation(line: 0, scope: !48)
!51 = !DILocation(line: 24, column: 10, scope: !52)
!52 = distinct !DILexicalBlock(scope: !48, file: !1, line: 23, column: 12)
!53 = !DILocation(line: 24, column: 14, scope: !54)
!54 = !DILexicalBlockFile(scope: !52, file: !1, discriminator: 1)
!55 = !DILocation(line: 24, column: 8, scope: !52)
!56 = !DILocation(line: 25, column: 10, scope: !52)
!57 = !DILocation(line: 25, column: 5, scope: !52)
!58 = !DILocation(line: 25, column: 8, scope: !52)
!59 = !DILocation(line: 26, column: 19, scope: !52)
!60 = !DILocation(line: 26, column: 25, scope: !52)
!61 = !DILocation(line: 26, column: 5, scope: !54)
!62 = !DILocation(line: 27, column: 3, scope: !48)
!63 = distinct !DISubprogram(name: "Vector", linkageName: "_ZN6VectorC1Eiii", scope: !12, file: !1, line: 11, type: !18, isLocal: false, isDefinition: true, scopeLine: 11, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !17, variables: !2)
!64 = !DILocalVariable(name: "this", arg: 1, scope: !63, type: !11, flags: DIFlagArtificial | DIFlagObjectPointer)
!65 = !DILocation(line: 0, scope: !63)
!66 = !DILocalVariable(name: "x", arg: 2, scope: !63, file: !1, line: 11, type: !10)
!67 = !DILocation(line: 11, column: 14, scope: !63)
!68 = !DILocalVariable(name: "y", arg: 3, scope: !63, file: !1, line: 11, type: !10)
!69 = !DILocation(line: 11, column: 21, scope: !63)
!70 = !DILocalVariable(name: "z", arg: 4, scope: !63, file: !1, line: 11, type: !10)
!71 = !DILocation(line: 11, column: 28, scope: !63)
!72 = !DILocation(line: 11, column: 62, scope: !63)
!73 = !DILocation(line: 11, column: 63, scope: !63)
!74 = distinct !DISubprogram(name: "Vector", linkageName: "_ZN6VectorC2Eiii", scope: !12, file: !1, line: 11, type: !18, isLocal: false, isDefinition: true, scopeLine: 11, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !17, variables: !2)
!75 = !DILocalVariable(name: "this", arg: 1, scope: !74, type: !11, flags: DIFlagArtificial | DIFlagObjectPointer)
!76 = !DILocation(line: 0, scope: !74)
!77 = !DILocalVariable(name: "x", arg: 2, scope: !74, file: !1, line: 11, type: !10)
!78 = !DILocation(line: 11, column: 14, scope: !74)
!79 = !DILocalVariable(name: "y", arg: 3, scope: !74, file: !1, line: 11, type: !10)
!80 = !DILocation(line: 11, column: 21, scope: !74)
!81 = !DILocalVariable(name: "z", arg: 4, scope: !74, file: !1, line: 11, type: !10)
!82 = !DILocation(line: 11, column: 28, scope: !74)
!83 = !DILocation(line: 11, column: 42, scope: !74)
!84 = !DILocation(line: 11, column: 49, scope: !74)
!85 = !DILocation(line: 11, column: 56, scope: !74)
!86 = !DILocation(line: 11, column: 63, scope: !74)
