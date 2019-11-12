; ModuleID = 'phi-2.pp.bc'
source_filename = "phi-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.foo = type { i32, %struct.bar, [10 x [10 x [9 x i32]]] }
%struct.bar = type { i32, float }

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; CHECK: define [4 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [37, 100, 10, 0], align 1
; CHECK: }
; CHECK: }

declare i32 @printf(i8*, ...) #2
; CHECK: declare si32 @ar.libc.printf(si8*, ...)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !8 {
  %3 = alloca %struct.foo, align 4
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, metadata !17, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.declare(metadata %struct.foo* %3, metadata !18, metadata !DIExpression()), !dbg !33
  call void @llvm.dbg.value(metadata i32 0, metadata !34, metadata !DIExpression()), !dbg !16
  br label %4, !dbg !35

4:                                                ; preds = %36, %2
  %.02 = phi i32 [ 0, %2 ], [ %37, %36 ], !dbg !37
  call void @llvm.dbg.value(metadata i32 %.02, metadata !34, metadata !DIExpression()), !dbg !16
  %5 = icmp slt i32 %.02, 10, !dbg !38
  br i1 %5, label %6, label %38, !dbg !40

6:                                                ; preds = %4
  call void @llvm.dbg.value(metadata i32 0, metadata !41, metadata !DIExpression()), !dbg !16
  br label %7, !dbg !42

7:                                                ; preds = %33, %6
  %.01 = phi i32 [ 0, %6 ], [ %34, %33 ], !dbg !45
  call void @llvm.dbg.value(metadata i32 %.01, metadata !41, metadata !DIExpression()), !dbg !16
  %8 = icmp slt i32 %.01, 10, !dbg !46
  br i1 %8, label %9, label %35, !dbg !48

9:                                                ; preds = %7
  call void @llvm.dbg.value(metadata i32 0, metadata !49, metadata !DIExpression()), !dbg !16
  br label %10, !dbg !50

10:                                               ; preds = %30, %9
  %.0 = phi i32 [ 0, %9 ], [ %31, %30 ], !dbg !53
  call void @llvm.dbg.value(metadata i32 %.0, metadata !49, metadata !DIExpression()), !dbg !16
  %11 = icmp slt i32 %.0, 9, !dbg !54
  br i1 %11, label %12, label %32, !dbg !56

12:                                               ; preds = %10
  %13 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 2, !dbg !57
  %14 = sext i32 %.02 to i64, !dbg !59
  %15 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %13, i64 0, i64 %14, !dbg !59
  %16 = sext i32 %.01 to i64, !dbg !59
  %17 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %15, i64 0, i64 %16, !dbg !59
  %18 = sext i32 %.0 to i64, !dbg !59
  %19 = getelementptr inbounds [9 x i32], [9 x i32]* %17, i64 0, i64 %18, !dbg !59
  store i32 %0, i32* %19, align 4, !dbg !60
  %20 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 2, !dbg !61
  %21 = sext i32 %.02 to i64, !dbg !62
  %22 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %20, i64 0, i64 %21, !dbg !62
  %23 = sext i32 %.01 to i64, !dbg !62
  %24 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %22, i64 0, i64 %23, !dbg !62
  %25 = sext i32 %.0 to i64, !dbg !62
  %26 = getelementptr inbounds [9 x i32], [9 x i32]* %24, i64 0, i64 %25, !dbg !62
  %27 = load i32, i32* %26, align 4, !dbg !62
  %28 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 1, !dbg !63
  %29 = getelementptr inbounds %struct.bar, %struct.bar* %28, i32 0, i32 0, !dbg !64
  store i32 %27, i32* %29, align 4, !dbg !65
  br label %30, !dbg !66

30:                                               ; preds = %12
  %31 = add nsw i32 %.0, 1, !dbg !67
  call void @llvm.dbg.value(metadata i32 %31, metadata !49, metadata !DIExpression()), !dbg !16
  br label %10, !dbg !68, !llvm.loop !69

32:                                               ; preds = %10
  br label %33, !dbg !71

33:                                               ; preds = %32
  %34 = add nsw i32 %.01, 1, !dbg !72
  call void @llvm.dbg.value(metadata i32 %34, metadata !41, metadata !DIExpression()), !dbg !16
  br label %7, !dbg !73, !llvm.loop !74

35:                                               ; preds = %7
  br label %36, !dbg !76

36:                                               ; preds = %35
  %37 = add nsw i32 %.02, 1, !dbg !77
  call void @llvm.dbg.value(metadata i32 %37, metadata !34, metadata !DIExpression()), !dbg !16
  br label %4, !dbg !78, !llvm.loop !79

38:                                               ; preds = %4
  call void @llvm.dbg.value(metadata i32 0, metadata !34, metadata !DIExpression()), !dbg !16
  br label %39, !dbg !81

39:                                               ; preds = %53, %38
  %.1 = phi i32 [ 0, %38 ], [ %54, %53 ], !dbg !83
  call void @llvm.dbg.value(metadata i32 %.1, metadata !34, metadata !DIExpression()), !dbg !16
  %40 = icmp slt i32 %.1, 10, !dbg !84
  br i1 %40, label %41, label %55, !dbg !86

41:                                               ; preds = %39
  %42 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 2, !dbg !87
  %43 = sext i32 %.1 to i64, !dbg !89
  %44 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %42, i64 0, i64 %43, !dbg !89
  %45 = sext i32 %.1 to i64, !dbg !89
  %46 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %44, i64 0, i64 %45, !dbg !89
  %47 = sub nsw i32 %.1, 1, !dbg !90
  %48 = sext i32 %47 to i64, !dbg !89
  %49 = getelementptr inbounds [9 x i32], [9 x i32]* %46, i64 0, i64 %48, !dbg !89
  %50 = load i32, i32* %49, align 4, !dbg !89
  %51 = getelementptr inbounds [4 x i8], [4 x i8]* @.str, i64 0, i64 0, !dbg !91
  %52 = call i32 (i8*, ...) @printf(i8* %51, i32 %50), !dbg !91
  br label %53, !dbg !92

53:                                               ; preds = %41
  %54 = add nsw i32 %.1, 1, !dbg !93
  call void @llvm.dbg.value(metadata i32 %54, metadata !34, metadata !DIExpression()), !dbg !16
  br label %39, !dbg !94, !llvm.loop !95

55:                                               ; preds = %39
  ret i32 0, !dbg !97
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}* $3 = allocate {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}, 1, align 4
; CHECK:   si32 %.02 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #8} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#5} {
; CHECK:   %.02 silt 10
; CHECK:   si32 %.01 = 0
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#6} {
; CHECK:   %.02 sige 10
; CHECK:   si32 %.1 = 0
; CHECK: }
; CHECK: #5 predecessors={#3, #13} successors={#7, #8} {
; CHECK: }
; CHECK: #7 predecessors={#5} successors={#11} {
; CHECK:   %.01 silt 10
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #8 predecessors={#5} successors={#2} {
; CHECK:   %.01 sige 10
; CHECK:   si32 %4 = %.02 sadd.nw 1
; CHECK:   si32 %.02 = %4
; CHECK: }
; CHECK: #6 predecessors={#4, #9} successors={#9, #10} {
; CHECK: }
; CHECK: #9 predecessors={#6} successors={#6} {
; CHECK:   %.1 silt 10
; CHECK:   [10 x [10 x [9 x si32]]]* %5 = ptrshift $3, 3612 * 0, 1 * 12
; CHECK:   si64 %6 = sext %.1
; CHECK:   [10 x [9 x si32]]* %7 = ptrshift %5, 3600 * 0, 360 * %6
; CHECK:   si64 %8 = sext %.1
; CHECK:   [9 x si32]* %9 = ptrshift %7, 360 * 0, 36 * %8
; CHECK:   si32 %10 = %.1 ssub.nw 1
; CHECK:   si64 %11 = sext %10
; CHECK:   si32* %12 = ptrshift %9, 36 * 0, 4 * %11
; CHECK:   si32 %13 = load %12, align 4
; CHECK:   si8* %14 = ptrshift @.str, 4 * 0, 1 * 0
; CHECK:   si32 %15 = call @ar.libc.printf(%14, %13)
; CHECK:   si32 %16 = %.1 sadd.nw 1
; CHECK:   si32 %.1 = %16
; CHECK: }
; CHECK: #10 !exit predecessors={#6} {
; CHECK:   %.1 sige 10
; CHECK:   return 0
; CHECK: }
; CHECK: #11 predecessors={#7, #12} successors={#12, #13} {
; CHECK: }
; CHECK: #12 predecessors={#11} successors={#11} {
; CHECK:   %.0 silt 9
; CHECK:   [10 x [10 x [9 x si32]]]* %17 = ptrshift $3, 3612 * 0, 1 * 12
; CHECK:   si64 %18 = sext %.02
; CHECK:   [10 x [9 x si32]]* %19 = ptrshift %17, 3600 * 0, 360 * %18
; CHECK:   si64 %20 = sext %.01
; CHECK:   [9 x si32]* %21 = ptrshift %19, 360 * 0, 36 * %20
; CHECK:   si64 %22 = sext %.0
; CHECK:   si32* %23 = ptrshift %21, 36 * 0, 4 * %22
; CHECK:   store %23, %1, align 4
; CHECK:   [10 x [10 x [9 x si32]]]* %24 = ptrshift $3, 3612 * 0, 1 * 12
; CHECK:   si64 %25 = sext %.02
; CHECK:   [10 x [9 x si32]]* %26 = ptrshift %24, 3600 * 0, 360 * %25
; CHECK:   si64 %27 = sext %.01
; CHECK:   [9 x si32]* %28 = ptrshift %26, 360 * 0, 36 * %27
; CHECK:   si64 %29 = sext %.0
; CHECK:   si32* %30 = ptrshift %28, 36 * 0, 4 * %29
; CHECK:   si32 %31 = load %30, align 4
; CHECK:   {0: si32, 4: float}* %32 = ptrshift $3, 3612 * 0, 1 * 4
; CHECK:   si32* %33 = ptrshift %32, 8 * 0, 1 * 0
; CHECK:   store %33, %31, align 4
; CHECK:   si32 %34 = %.0 sadd.nw 1
; CHECK:   si32 %.0 = %34
; CHECK: }
; CHECK: #13 predecessors={#11} successors={#5} {
; CHECK:   %.0 sige 9
; CHECK:   si32 %35 = %.01 sadd.nw 1
; CHECK:   si32 %.01 = %35
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "phi-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 18, type: !9, scopeLine: 18, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!15 = !DILocalVariable(name: "argc", arg: 1, scope: !8, file: !1, line: 18, type: !11)
!16 = !DILocation(line: 0, scope: !8)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !8, file: !1, line: 18, type: !12)
!18 = !DILocalVariable(name: "x", scope: !8, file: !1, line: 20, type: !19)
!19 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "foo", file: !1, line: 11, size: 28896, elements: !20)
!20 = !{!21, !22, !28}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !19, file: !1, line: 12, baseType: !11, size: 32)
!22 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !19, file: !1, line: 13, baseType: !23, size: 64, offset: 32)
!23 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bar", file: !1, line: 6, size: 64, elements: !24)
!24 = !{!25, !26}
!25 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !23, file: !1, line: 7, baseType: !11, size: 32)
!26 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !23, file: !1, line: 8, baseType: !27, size: 32, offset: 32)
!27 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!28 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !19, file: !1, line: 14, baseType: !29, size: 28800, offset: 96)
!29 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 28800, elements: !30)
!30 = !{!31, !31, !32}
!31 = !DISubrange(count: 10)
!32 = !DISubrange(count: 9)
!33 = !DILocation(line: 20, column: 14, scope: !8)
!34 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 19, type: !11)
!35 = !DILocation(line: 21, column: 8, scope: !36)
!36 = distinct !DILexicalBlock(scope: !8, file: !1, line: 21, column: 3)
!37 = !DILocation(line: 0, scope: !36)
!38 = !DILocation(line: 21, column: 17, scope: !39)
!39 = distinct !DILexicalBlock(scope: !36, file: !1, line: 21, column: 3)
!40 = !DILocation(line: 21, column: 3, scope: !36)
!41 = !DILocalVariable(name: "j", scope: !8, file: !1, line: 19, type: !11)
!42 = !DILocation(line: 22, column: 10, scope: !43)
!43 = distinct !DILexicalBlock(scope: !44, file: !1, line: 22, column: 5)
!44 = distinct !DILexicalBlock(scope: !39, file: !1, line: 21, column: 35)
!45 = !DILocation(line: 0, scope: !43)
!46 = !DILocation(line: 22, column: 19, scope: !47)
!47 = distinct !DILexicalBlock(scope: !43, file: !1, line: 22, column: 5)
!48 = !DILocation(line: 22, column: 5, scope: !43)
!49 = !DILocalVariable(name: "k", scope: !8, file: !1, line: 19, type: !11)
!50 = !DILocation(line: 23, column: 12, scope: !51)
!51 = distinct !DILexicalBlock(scope: !52, file: !1, line: 23, column: 7)
!52 = distinct !DILexicalBlock(scope: !47, file: !1, line: 22, column: 37)
!53 = !DILocation(line: 0, scope: !51)
!54 = !DILocation(line: 23, column: 21, scope: !55)
!55 = distinct !DILexicalBlock(scope: !51, file: !1, line: 23, column: 7)
!56 = !DILocation(line: 23, column: 7, scope: !51)
!57 = !DILocation(line: 24, column: 11, scope: !58)
!58 = distinct !DILexicalBlock(scope: !55, file: !1, line: 23, column: 43)
!59 = !DILocation(line: 24, column: 9, scope: !58)
!60 = !DILocation(line: 24, column: 22, scope: !58)
!61 = !DILocation(line: 25, column: 19, scope: !58)
!62 = !DILocation(line: 25, column: 17, scope: !58)
!63 = !DILocation(line: 25, column: 11, scope: !58)
!64 = !DILocation(line: 25, column: 13, scope: !58)
!65 = !DILocation(line: 25, column: 15, scope: !58)
!66 = !DILocation(line: 26, column: 7, scope: !58)
!67 = !DILocation(line: 23, column: 39, scope: !55)
!68 = !DILocation(line: 23, column: 7, scope: !55)
!69 = distinct !{!69, !56, !70}
!70 = !DILocation(line: 26, column: 7, scope: !51)
!71 = !DILocation(line: 27, column: 5, scope: !52)
!72 = !DILocation(line: 22, column: 33, scope: !47)
!73 = !DILocation(line: 22, column: 5, scope: !47)
!74 = distinct !{!74, !48, !75}
!75 = !DILocation(line: 27, column: 5, scope: !43)
!76 = !DILocation(line: 28, column: 3, scope: !44)
!77 = !DILocation(line: 21, column: 31, scope: !39)
!78 = !DILocation(line: 21, column: 3, scope: !39)
!79 = distinct !{!79, !40, !80}
!80 = !DILocation(line: 28, column: 3, scope: !36)
!81 = !DILocation(line: 30, column: 8, scope: !82)
!82 = distinct !DILexicalBlock(scope: !8, file: !1, line: 30, column: 3)
!83 = !DILocation(line: 0, scope: !82)
!84 = !DILocation(line: 30, column: 17, scope: !85)
!85 = distinct !DILexicalBlock(scope: !82, file: !1, line: 30, column: 3)
!86 = !DILocation(line: 30, column: 3, scope: !82)
!87 = !DILocation(line: 31, column: 22, scope: !88)
!88 = distinct !DILexicalBlock(scope: !85, file: !1, line: 30, column: 35)
!89 = !DILocation(line: 31, column: 20, scope: !88)
!90 = !DILocation(line: 31, column: 32, scope: !88)
!91 = !DILocation(line: 31, column: 5, scope: !88)
!92 = !DILocation(line: 32, column: 3, scope: !88)
!93 = !DILocation(line: 30, column: 31, scope: !85)
!94 = !DILocation(line: 30, column: 3, scope: !85)
!95 = distinct !{!95, !86, !96}
!96 = !DILocation(line: 32, column: 3, scope: !82)
!97 = !DILocation(line: 34, column: 3, scope: !8)
