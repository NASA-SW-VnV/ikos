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
  %2 = alloca %class.Vector*, align 8
  store %class.Vector* %0, %class.Vector** %2, align 8
  call void @llvm.dbg.declare(metadata %class.Vector** %2, metadata !22, metadata !DIExpression()), !dbg !23
  %3 = load %class.Vector*, %class.Vector** %2, align 8, !dbg !24
  %4 = getelementptr inbounds %class.Vector, %class.Vector* %3, i32 0, i32 1, !dbg !25
  %5 = load i32, i32* %4, align 4, !dbg !25
  ret i32 %5, !dbg !26
}
; CHECK: define si32 @_Z1fP6Vector({0: si32, 4: si32, 8: si32}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si32, 4: si32, 8: si32}** $2 = allocate {0: si32, 4: si32, 8: si32}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: si32, 4: si32, 8: si32}** %3 = bitcast $2
; CHECK:   {0: si32, 4: si32, 8: si32}* %4 = load %3, align 8
; CHECK:   si32* %5 = ptrshift %4, 12 * 0, 1 * 4
; CHECK:   si32 %6 = load %5, align 4
; CHECK:   return %6
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN6MasterC1Ev(%class.Master*) unnamed_addr #3 align 2 !dbg !42 {
  %2 = alloca %class.Master*, align 8
  store %class.Master* %0, %class.Master** %2, align 8
  call void @llvm.dbg.declare(metadata %class.Master** %2, metadata !43, metadata !DIExpression()), !dbg !45
  %3 = load %class.Master*, %class.Master** %2, align 8
  call void @_ZN6MasterC2Ev(%class.Master* %3), !dbg !46
  ret void, !dbg !47
}
; CHECK: define void @_ZN6MasterC1Ev({0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}** $2 = allocate {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* %3 = load $2, align 8
; CHECK:   call @_ZN6MasterC2Ev(%3)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define linkonce_odr void @_ZN6MasterC2Ev(%class.Master*) unnamed_addr #3 align 2 !dbg !48 {
  %2 = alloca %class.Master*, align 8
  store %class.Master* %0, %class.Master** %2, align 8
  call void @llvm.dbg.declare(metadata %class.Master** %2, metadata !49, metadata !DIExpression()), !dbg !50
  %3 = load %class.Master*, %class.Master** %2, align 8
  %4 = call i8* @_Znwm(i64 12) #6, !dbg !51
  %5 = bitcast i8* %4 to %class.Vector*, !dbg !51
  call void @_ZN6VectorC1Eiii(%class.Vector* %5, i32 1, i32 2, i32 3) #7, !dbg !53
  %6 = getelementptr inbounds %class.Master, %class.Master* %3, i32 0, i32 0, !dbg !54
  store %class.Vector* %5, %class.Vector** %6, align 8, !dbg !55
  %7 = call i8* @_Znwm(i64 4) #6, !dbg !56
  %8 = bitcast i8* %7 to i32*, !dbg !56
  store i32 4, i32* %8, align 4, !dbg !56
  %9 = getelementptr inbounds %class.Master, %class.Master* %3, i32 0, i32 1, !dbg !57
  store i32* %8, i32** %9, align 8, !dbg !58
  %10 = getelementptr inbounds %class.Master, %class.Master* %3, i32 0, i32 0, !dbg !59
  %11 = load %class.Vector*, %class.Vector** %10, align 8, !dbg !59
  %12 = call i32 @_Z1fP6Vector(%class.Vector* %11), !dbg !60
  %13 = icmp eq i32 %12, 2, !dbg !61
  %14 = zext i1 %13 to i32, !dbg !60
  call void @__ikos_assert(i32 %14), !dbg !62
  ret void, !dbg !63
}
; CHECK: define void @_ZN6MasterC2Ev({0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* %1) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}** $2 = allocate {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}** %3 = bitcast $2
; CHECK:   {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* %4 = load %3, align 8
; CHECK:   si8* %5 = call @ar.libcpp.new(12)
; CHECK:   {0: si32, 4: si32, 8: si32}* %6 = bitcast %5
; CHECK:   call @_ZN6VectorC1Eiii(%6, 1, 2, 3)
; CHECK:   {0: si32, 4: si32, 8: si32}** %7 = ptrshift %4, 16 * 0, 1 * 0
; CHECK:   store %7, %6, align 8
; CHECK:   si8* %8 = call @ar.libcpp.new(4)
; CHECK:   si32* %9 = bitcast %8
; CHECK:   store %9, 4, align 4
; CHECK:   si32** %10 = ptrshift %4, 16 * 0, 1 * 8
; CHECK:   store %10, %9, align 8
; CHECK:   {0: si32, 4: si32, 8: si32}** %11 = ptrshift %4, 16 * 0, 1 * 0
; CHECK:   {0: si32, 4: si32, 8: si32}** %12 = bitcast %11
; CHECK:   {0: si32, 4: si32, 8: si32}* %13 = load %12, align 8
; CHECK:   si32 %14 = call @_Z1fP6Vector(%13)
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   %14 sieq 2
; CHECK:   ui1 %15 = 1
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   %14 sine 2
; CHECK:   ui1 %15 = 0
; CHECK: }
; CHECK: #4 !exit predecessors={#2, #3} {
; CHECK:   ui32 %16 = zext %15
; CHECK:   call @ar.ikos.assert(%16)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN6VectorC1Eiii(%class.Vector*, i32, i32, i32) unnamed_addr #0 align 2 !dbg !64 {
  %5 = alloca %class.Vector*, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store %class.Vector* %0, %class.Vector** %5, align 8
  call void @llvm.dbg.declare(metadata %class.Vector** %5, metadata !65, metadata !DIExpression()), !dbg !66
  store i32 %1, i32* %6, align 4
  call void @llvm.dbg.declare(metadata i32* %6, metadata !67, metadata !DIExpression()), !dbg !68
  store i32 %2, i32* %7, align 4
  call void @llvm.dbg.declare(metadata i32* %7, metadata !69, metadata !DIExpression()), !dbg !70
  store i32 %3, i32* %8, align 4
  call void @llvm.dbg.declare(metadata i32* %8, metadata !71, metadata !DIExpression()), !dbg !72
  %9 = load %class.Vector*, %class.Vector** %5, align 8
  %10 = load i32, i32* %6, align 4, !dbg !73
  %11 = load i32, i32* %7, align 4, !dbg !73
  %12 = load i32, i32* %8, align 4, !dbg !73
  call void @_ZN6VectorC2Eiii(%class.Vector* %9, i32 %10, i32 %11, i32 %12) #7, !dbg !73
  ret void, !dbg !74
}
; CHECK: define void @_ZN6VectorC1Eiii({0: si32, 4: si32, 8: si32}* %1, si32 %2, si32 %3, si32 %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si32, 4: si32, 8: si32}** $5 = allocate {0: si32, 4: si32, 8: si32}*, 1, align 8
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   si32* $7 = allocate si32, 1, align 4
; CHECK:   si32* $8 = allocate si32, 1, align 4
; CHECK:   store $5, %1, align 8
; CHECK:   store $6, %2, align 4
; CHECK:   store $7, %3, align 4
; CHECK:   store $8, %4, align 4
; CHECK:   {0: si32, 4: si32, 8: si32}* %9 = load $5, align 8
; CHECK:   si32 %10 = load $6, align 4
; CHECK:   si32 %11 = load $7, align 4
; CHECK:   si32 %12 = load $8, align 4
; CHECK:   call @_ZN6VectorC2Eiii(%9, %10, %11, %12)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN6VectorC2Eiii(%class.Vector*, i32, i32, i32) unnamed_addr #0 align 2 !dbg !75 {
  %5 = alloca %class.Vector*, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  store %class.Vector* %0, %class.Vector** %5, align 8
  call void @llvm.dbg.declare(metadata %class.Vector** %5, metadata !76, metadata !DIExpression()), !dbg !77
  store i32 %1, i32* %6, align 4
  call void @llvm.dbg.declare(metadata i32* %6, metadata !78, metadata !DIExpression()), !dbg !79
  store i32 %2, i32* %7, align 4
  call void @llvm.dbg.declare(metadata i32* %7, metadata !80, metadata !DIExpression()), !dbg !81
  store i32 %3, i32* %8, align 4
  call void @llvm.dbg.declare(metadata i32* %8, metadata !82, metadata !DIExpression()), !dbg !83
  %9 = load %class.Vector*, %class.Vector** %5, align 8
  %10 = getelementptr inbounds %class.Vector, %class.Vector* %9, i32 0, i32 0, !dbg !84
  %11 = load i32, i32* %6, align 4, !dbg !85
  store i32 %11, i32* %10, align 4, !dbg !84
  %12 = getelementptr inbounds %class.Vector, %class.Vector* %9, i32 0, i32 1, !dbg !86
  %13 = load i32, i32* %7, align 4, !dbg !87
  store i32 %13, i32* %12, align 4, !dbg !86
  %14 = getelementptr inbounds %class.Vector, %class.Vector* %9, i32 0, i32 2, !dbg !88
  %15 = load i32, i32* %8, align 4, !dbg !89
  store i32 %15, i32* %14, align 4, !dbg !88
  ret void, !dbg !90
}
; CHECK: define void @_ZN6VectorC2Eiii({0: si32, 4: si32, 8: si32}* %1, si32 %2, si32 %3, si32 %4) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si32, 4: si32, 8: si32}** $5 = allocate {0: si32, 4: si32, 8: si32}*, 1, align 8
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   si32* $7 = allocate si32, 1, align 4
; CHECK:   si32* $8 = allocate si32, 1, align 4
; CHECK:   store $5, %1, align 8
; CHECK:   store $6, %2, align 4
; CHECK:   store $7, %3, align 4
; CHECK:   store $8, %4, align 4
; CHECK:   {0: si32, 4: si32, 8: si32}** %9 = bitcast $5
; CHECK:   {0: si32, 4: si32, 8: si32}* %10 = load %9, align 8
; CHECK:   si32* %11 = ptrshift %10, 12 * 0, 1 * 0
; CHECK:   si32 %12 = load $6, align 4
; CHECK:   store %11, %12, align 4
; CHECK:   si32* %13 = ptrshift %10, 12 * 0, 1 * 4
; CHECK:   si32 %14 = load $7, align 4
; CHECK:   store %13, %14, align 4
; CHECK:   si32* %15 = ptrshift %10, 12 * 0, 1 * 8
; CHECK:   si32 %16 = load $8, align 4
; CHECK:   store %15, %16, align 4
; CHECK:   return
; CHECK: }
; CHECK: }

declare void @__ikos_assert(i32) #5
; CHECK: declare void @ar.ikos.assert(ui32)

; Function Attrs: nobuiltin
declare noalias i8* @_Znwm(i64) #4
; CHECK: declare si8* @ar.libcpp.new(ui64)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main() #2 !dbg !27 {
  %1 = alloca i32, align 4
  %2 = alloca %class.Master, align 8
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata %class.Master* %2, metadata !30, metadata !DIExpression()), !dbg !40
  call void @_ZN6MasterC1Ev(%class.Master* %2), !dbg !40
  ret i32 0, !dbg !41
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}* $2 = allocate {0: {0: si32, 4: si32, 8: si32}*, 8: si32*}, 1, align 8
; CHECK:   store $1, 0, align 4
; CHECK:   call @_ZN6MasterC1Ev($2)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

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
!1 = !DIFile(filename: "constructors.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!23 = !DILocation(line: 14, column: 15, scope: !8)
!24 = !DILocation(line: 15, column: 10, scope: !8)
!25 = !DILocation(line: 15, column: 13, scope: !8)
!26 = !DILocation(line: 15, column: 3, scope: !8)
!27 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 30, type: !28, scopeLine: 30, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!28 = !DISubroutineType(types: !29)
!29 = !{!11}
!30 = !DILocalVariable(name: "master", scope: !27, file: !1, line: 31, type: !31)
!31 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Master", file: !1, line: 18, size: 128, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !32, identifier: "_ZTS6Master")
!32 = !{!33, !34, !36}
!33 = !DIDerivedType(tag: DW_TAG_member, name: "_v", scope: !31, file: !1, line: 20, baseType: !12, size: 64, flags: DIFlagPublic)
!34 = !DIDerivedType(tag: DW_TAG_member, name: "_p", scope: !31, file: !1, line: 21, baseType: !35, size: 64, offset: 64, flags: DIFlagPublic)
!35 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!36 = !DISubprogram(name: "Master", scope: !31, file: !1, line: 23, type: !37, scopeLine: 23, flags: DIFlagPublic | DIFlagPrototyped, spFlags: 0)
!37 = !DISubroutineType(types: !38)
!38 = !{null, !39}
!39 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !31, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!40 = !DILocation(line: 31, column: 10, scope: !27)
!41 = !DILocation(line: 32, column: 3, scope: !27)
!42 = distinct !DISubprogram(name: "Master", linkageName: "_ZN6MasterC1Ev", scope: !31, file: !1, line: 23, type: !37, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !36, retainedNodes: !2)
!43 = !DILocalVariable(name: "this", arg: 1, scope: !42, type: !44, flags: DIFlagArtificial | DIFlagObjectPointer)
!44 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !31, size: 64)
!45 = !DILocation(line: 0, scope: !42)
!46 = !DILocation(line: 23, column: 12, scope: !42)
!47 = !DILocation(line: 27, column: 3, scope: !42)
!48 = distinct !DISubprogram(name: "Master", linkageName: "_ZN6MasterC2Ev", scope: !31, file: !1, line: 23, type: !37, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !36, retainedNodes: !2)
!49 = !DILocalVariable(name: "this", arg: 1, scope: !48, type: !44, flags: DIFlagArtificial | DIFlagObjectPointer)
!50 = !DILocation(line: 0, scope: !48)
!51 = !DILocation(line: 24, column: 10, scope: !52)
!52 = distinct !DILexicalBlock(scope: !48, file: !1, line: 23, column: 12)
!53 = !DILocation(line: 24, column: 14, scope: !52)
!54 = !DILocation(line: 24, column: 5, scope: !52)
!55 = !DILocation(line: 24, column: 8, scope: !52)
!56 = !DILocation(line: 25, column: 10, scope: !52)
!57 = !DILocation(line: 25, column: 5, scope: !52)
!58 = !DILocation(line: 25, column: 8, scope: !52)
!59 = !DILocation(line: 26, column: 21, scope: !52)
!60 = !DILocation(line: 26, column: 19, scope: !52)
!61 = !DILocation(line: 26, column: 25, scope: !52)
!62 = !DILocation(line: 26, column: 5, scope: !52)
!63 = !DILocation(line: 27, column: 3, scope: !48)
!64 = distinct !DISubprogram(name: "Vector", linkageName: "_ZN6VectorC1Eiii", scope: !13, file: !1, line: 11, type: !19, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !18, retainedNodes: !2)
!65 = !DILocalVariable(name: "this", arg: 1, scope: !64, type: !12, flags: DIFlagArtificial | DIFlagObjectPointer)
!66 = !DILocation(line: 0, scope: !64)
!67 = !DILocalVariable(name: "x", arg: 2, scope: !64, file: !1, line: 11, type: !11)
!68 = !DILocation(line: 11, column: 14, scope: !64)
!69 = !DILocalVariable(name: "y", arg: 3, scope: !64, file: !1, line: 11, type: !11)
!70 = !DILocation(line: 11, column: 21, scope: !64)
!71 = !DILocalVariable(name: "z", arg: 4, scope: !64, file: !1, line: 11, type: !11)
!72 = !DILocation(line: 11, column: 28, scope: !64)
!73 = !DILocation(line: 11, column: 62, scope: !64)
!74 = !DILocation(line: 11, column: 63, scope: !64)
!75 = distinct !DISubprogram(name: "Vector", linkageName: "_ZN6VectorC2Eiii", scope: !13, file: !1, line: 11, type: !19, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !18, retainedNodes: !2)
!76 = !DILocalVariable(name: "this", arg: 1, scope: !75, type: !12, flags: DIFlagArtificial | DIFlagObjectPointer)
!77 = !DILocation(line: 0, scope: !75)
!78 = !DILocalVariable(name: "x", arg: 2, scope: !75, file: !1, line: 11, type: !11)
!79 = !DILocation(line: 11, column: 14, scope: !75)
!80 = !DILocalVariable(name: "y", arg: 3, scope: !75, file: !1, line: 11, type: !11)
!81 = !DILocation(line: 11, column: 21, scope: !75)
!82 = !DILocalVariable(name: "z", arg: 4, scope: !75, file: !1, line: 11, type: !11)
!83 = !DILocation(line: 11, column: 28, scope: !75)
!84 = !DILocation(line: 11, column: 42, scope: !75)
!85 = !DILocation(line: 11, column: 45, scope: !75)
!86 = !DILocation(line: 11, column: 49, scope: !75)
!87 = !DILocation(line: 11, column: 52, scope: !75)
!88 = !DILocation(line: 11, column: 56, scope: !75)
!89 = !DILocation(line: 11, column: 59, scope: !75)
!90 = !DILocation(line: 11, column: 63, scope: !75)
