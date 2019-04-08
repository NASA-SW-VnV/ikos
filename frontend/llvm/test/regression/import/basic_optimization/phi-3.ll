; ModuleID = 'phi-3.pp.bc'
source_filename = "phi-3.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

declare void @__ikos_assert(i32) #2
; CHECK: declare void @ar.ikos.assert(ui32)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @foo(i32*, i32*, i32*) #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i32* %0, metadata !13, metadata !DIExpression()), !dbg !14
  call void @llvm.dbg.value(metadata i32* %1, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i32* %2, metadata !17, metadata !DIExpression()), !dbg !18
  %4 = getelementptr inbounds i32, i32* %0, i64 1, !dbg !19
  call void @llvm.dbg.value(metadata i32* %4, metadata !20, metadata !DIExpression()), !dbg !21
  %5 = getelementptr inbounds i32, i32* %1, i64 2, !dbg !22
  call void @llvm.dbg.value(metadata i32* %5, metadata !23, metadata !DIExpression()), !dbg !24
  %6 = icmp eq i32* %4, %5, !dbg !25
  br i1 %6, label %7, label %10, !dbg !27

; <label>:7:                                      ; preds = %3
  %8 = getelementptr inbounds i32, i32* %5, i64 -10, !dbg !28
  call void @llvm.dbg.value(metadata i32* %8, metadata !23, metadata !DIExpression()), !dbg !24
  %9 = getelementptr inbounds i32, i32* %4, i64 42, !dbg !30
  call void @llvm.dbg.value(metadata i32* %9, metadata !20, metadata !DIExpression()), !dbg !21
  br label %10, !dbg !31

; <label>:10:                                     ; preds = %7, %3
  %.01 = phi i32* [ %9, %7 ], [ %4, %3 ], !dbg !32
  %.0 = phi i32* [ %8, %7 ], [ %5, %3 ], !dbg !32
  call void @llvm.dbg.value(metadata i32* %.0, metadata !23, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.value(metadata i32* %.01, metadata !20, metadata !DIExpression()), !dbg !21
  %11 = load i32, i32* %.01, align 4, !dbg !33
  %12 = icmp eq i32 %11, 3, !dbg !34
  %13 = zext i1 %12 to i32, !dbg !34
  call void @__ikos_assert(i32 %13), !dbg !35
  %14 = load i32, i32* %.0, align 4, !dbg !36
  %15 = icmp eq i32 %14, 6, !dbg !37
  %16 = zext i1 %15 to i32, !dbg !37
  call void @__ikos_assert(i32 %16), !dbg !38
  %17 = load i32, i32* %.01, align 4, !dbg !39
  %18 = load i32, i32* %.0, align 4, !dbg !40
  %19 = add nsw i32 %17, %18, !dbg !41
  %20 = sext i32 %19 to i64, !dbg !42
  %21 = getelementptr inbounds i32, i32* %2, i64 %20, !dbg !42
  %22 = load i32, i32* %21, align 4, !dbg !42
  call void @llvm.dbg.value(metadata i32 %22, metadata !43, metadata !DIExpression()), !dbg !44
  %23 = load i32, i32* %.01, align 4, !dbg !45
  %24 = load i32, i32* %.0, align 4, !dbg !46
  %25 = add nsw i32 %23, %24, !dbg !47
  %26 = sext i32 %25 to i64, !dbg !48
  %27 = getelementptr inbounds i32, i32* %2, i64 %26, !dbg !48
  store i32 555, i32* %27, align 4, !dbg !49
  ret i32 %22, !dbg !50
}
; CHECK: define si32 @foo(si32* %1, si32* %2, si32* %3) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32* %4 = ptrshift %1, 4 * 1
; CHECK:   si32* %5 = ptrshift %2, 4 * 2
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   %4 peq %5
; CHECK:   si32* %6 = ptrshift %5, 4 * -10
; CHECK:   si32* %7 = ptrshift %4, 4 * 42
; CHECK:   si32* %.01 = %7
; CHECK:   si32* %.0 = %6
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   %4 pne %5
; CHECK:   si32* %.01 = %4
; CHECK:   si32* %.0 = %5
; CHECK: }
; CHECK: #4 predecessors={#2, #3} successors={#5, #6} {
; CHECK:   si32 %8 = load %.01, align 4
; CHECK: }
; CHECK: #5 predecessors={#4} successors={#7} {
; CHECK:   %8 sieq 3
; CHECK:   ui1 %9 = 1
; CHECK: }
; CHECK: #6 predecessors={#4} successors={#7} {
; CHECK:   %8 sine 3
; CHECK:   ui1 %9 = 0
; CHECK: }
; CHECK: #7 predecessors={#5, #6} successors={#8, #9} {
; CHECK:   ui32 %10 = zext %9
; CHECK:   call @ar.ikos.assert(%10)
; CHECK:   si32 %11 = load %.0, align 4
; CHECK: }
; CHECK: #8 predecessors={#7} successors={#10} {
; CHECK:   %11 sieq 6
; CHECK:   ui1 %12 = 1
; CHECK: }
; CHECK: #9 predecessors={#7} successors={#10} {
; CHECK:   %11 sine 6
; CHECK:   ui1 %12 = 0
; CHECK: }
; CHECK: #10 !exit predecessors={#8, #9} {
; CHECK:   ui32 %13 = zext %12
; CHECK:   call @ar.ikos.assert(%13)
; CHECK:   si32 %14 = load %.01, align 4
; CHECK:   si32 %15 = load %.0, align 4
; CHECK:   si32 %16 = %14 sadd.nw %15
; CHECK:   si64 %17 = sext %16
; CHECK:   si32* %18 = ptrshift %3, 4 * %17
; CHECK:   si32 %19 = load %18, align 4
; CHECK:   si32 %20 = load %.01, align 4
; CHECK:   si32 %21 = load %.0, align 4
; CHECK:   si32 %22 = %20 sadd.nw %21
; CHECK:   si64 %23 = sext %22
; CHECK:   si32* %24 = ptrshift %3, 4 * %23
; CHECK:   store %24, 555, align 4
; CHECK:   return %19
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !51 {
  %3 = alloca [2 x i32], align 4
  %4 = alloca [3 x i32], align 4
  %5 = alloca [10 x i32], align 16
  call void @llvm.dbg.value(metadata i32 %0, metadata !57, metadata !DIExpression()), !dbg !58
  call void @llvm.dbg.value(metadata i8** %1, metadata !59, metadata !DIExpression()), !dbg !60
  call void @llvm.dbg.declare(metadata [2 x i32]* %3, metadata !61, metadata !DIExpression()), !dbg !65
  call void @llvm.dbg.declare(metadata [3 x i32]* %4, metadata !66, metadata !DIExpression()), !dbg !70
  call void @llvm.dbg.declare(metadata [10 x i32]* %5, metadata !71, metadata !DIExpression()), !dbg !75
  %6 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i64 0, i64 9, !dbg !76
  store i32 666, i32* %6, align 4, !dbg !77
  %7 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 0, !dbg !78
  store i32 1, i32* %7, align 4, !dbg !79
  %8 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 1, !dbg !80
  store i32 3, i32* %8, align 4, !dbg !81
  %9 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 0, !dbg !82
  store i32 4, i32* %9, align 4, !dbg !83
  %10 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 1, !dbg !84
  store i32 5, i32* %10, align 4, !dbg !85
  %11 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 2, !dbg !86
  store i32 6, i32* %11, align 4, !dbg !87
  %12 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i32 0, i32 0, !dbg !88
  %13 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i32 0, i32 0, !dbg !89
  %14 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i32 0, i32 0, !dbg !90
  %15 = call i32 @foo(i32* %12, i32* %13, i32* %14), !dbg !91
  call void @llvm.dbg.value(metadata i32 %15, metadata !92, metadata !DIExpression()), !dbg !93
  %16 = icmp eq i32 %15, 666, !dbg !94
  %17 = zext i1 %16 to i32, !dbg !94
  call void @__ikos_assert(i32 %17), !dbg !95
  %18 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i64 0, i64 9, !dbg !96
  %19 = load i32, i32* %18, align 4, !dbg !96
  %20 = icmp eq i32 %19, 555, !dbg !97
  %21 = zext i1 %20 to i32, !dbg !97
  call void @__ikos_assert(i32 %21), !dbg !98
  ret i32 %15, !dbg !99
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   [2 x si32]* $3 = allocate [2 x si32], 1, align 4
; CHECK:   [3 x si32]* $4 = allocate [3 x si32], 1, align 4
; CHECK:   [10 x si32]* $5 = allocate [10 x si32], 1, align 16
; CHECK:   si32* %6 = ptrshift $5, 40 * 0, 4 * 9
; CHECK:   store %6, 666, align 4
; CHECK:   si32* %7 = ptrshift $3, 8 * 0, 4 * 0
; CHECK:   store %7, 1, align 4
; CHECK:   si32* %8 = ptrshift $3, 8 * 0, 4 * 1
; CHECK:   store %8, 3, align 4
; CHECK:   si32* %9 = ptrshift $4, 12 * 0, 4 * 0
; CHECK:   store %9, 4, align 4
; CHECK:   si32* %10 = ptrshift $4, 12 * 0, 4 * 1
; CHECK:   store %10, 5, align 4
; CHECK:   si32* %11 = ptrshift $4, 12 * 0, 4 * 2
; CHECK:   store %11, 6, align 4
; CHECK:   si32* %12 = ptrshift $3, 8 * 0, 4 * 0
; CHECK:   si32* %13 = ptrshift $4, 12 * 0, 4 * 0
; CHECK:   si32* %14 = ptrshift $5, 40 * 0, 4 * 0
; CHECK:   si32 %15 = call @foo(%12, %13, %14)
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   %15 sieq 666
; CHECK:   ui1 %16 = 1
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   %15 sine 666
; CHECK:   ui1 %16 = 0
; CHECK: }
; CHECK: #4 predecessors={#2, #3} successors={#5, #6} {
; CHECK:   ui32 %17 = zext %16
; CHECK:   call @ar.ikos.assert(%17)
; CHECK:   si32* %18 = ptrshift $5, 40 * 0, 4 * 9
; CHECK:   si32 %19 = load %18, align 4
; CHECK: }
; CHECK: #5 predecessors={#4} successors={#7} {
; CHECK:   %19 sieq 555
; CHECK:   ui1 %20 = 1
; CHECK: }
; CHECK: #6 predecessors={#4} successors={#7} {
; CHECK:   %19 sine 555
; CHECK:   ui1 %20 = 0
; CHECK: }
; CHECK: #7 !exit predecessors={#5, #6} {
; CHECK:   ui32 %21 = zext %20
; CHECK:   call @ar.ikos.assert(%21)
; CHECK:   return %15
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 (tags/RELEASE_800/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "phi-3.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 8.0.0 (tags/RELEASE_800/final)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 6, type: !9, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !12, !12, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!13 = !DILocalVariable(name: "a", arg: 1, scope: !8, file: !1, line: 6, type: !12)
!14 = !DILocation(line: 6, column: 14, scope: !8)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !8, file: !1, line: 6, type: !12)
!16 = !DILocation(line: 6, column: 22, scope: !8)
!17 = !DILocalVariable(name: "c", arg: 3, scope: !8, file: !1, line: 6, type: !12)
!18 = !DILocation(line: 6, column: 30, scope: !8)
!19 = !DILocation(line: 10, column: 9, scope: !8)
!20 = !DILocalVariable(name: "p", scope: !8, file: !1, line: 7, type: !12)
!21 = !DILocation(line: 7, column: 8, scope: !8)
!22 = !DILocation(line: 11, column: 9, scope: !8)
!23 = !DILocalVariable(name: "q", scope: !8, file: !1, line: 8, type: !12)
!24 = !DILocation(line: 8, column: 8, scope: !8)
!25 = !DILocation(line: 13, column: 9, scope: !26)
!26 = distinct !DILexicalBlock(scope: !8, file: !1, line: 13, column: 7)
!27 = !DILocation(line: 13, column: 7, scope: !8)
!28 = !DILocation(line: 14, column: 11, scope: !29)
!29 = distinct !DILexicalBlock(scope: !26, file: !1, line: 13, column: 15)
!30 = !DILocation(line: 15, column: 11, scope: !29)
!31 = !DILocation(line: 16, column: 3, scope: !29)
!32 = !DILocation(line: 0, scope: !8)
!33 = !DILocation(line: 17, column: 17, scope: !8)
!34 = !DILocation(line: 17, column: 20, scope: !8)
!35 = !DILocation(line: 17, column: 3, scope: !8)
!36 = !DILocation(line: 18, column: 17, scope: !8)
!37 = !DILocation(line: 18, column: 20, scope: !8)
!38 = !DILocation(line: 18, column: 3, scope: !8)
!39 = !DILocation(line: 20, column: 15, scope: !8)
!40 = !DILocation(line: 20, column: 20, scope: !8)
!41 = !DILocation(line: 20, column: 18, scope: !8)
!42 = !DILocation(line: 20, column: 13, scope: !8)
!43 = !DILocalVariable(name: "res", scope: !8, file: !1, line: 20, type: !11)
!44 = !DILocation(line: 20, column: 7, scope: !8)
!45 = !DILocation(line: 21, column: 5, scope: !8)
!46 = !DILocation(line: 21, column: 10, scope: !8)
!47 = !DILocation(line: 21, column: 8, scope: !8)
!48 = !DILocation(line: 21, column: 3, scope: !8)
!49 = !DILocation(line: 21, column: 14, scope: !8)
!50 = !DILocation(line: 22, column: 3, scope: !8)
!51 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 25, type: !52, scopeLine: 25, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!52 = !DISubroutineType(types: !53)
!53 = !{!11, !11, !54}
!54 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !55, size: 64)
!55 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !56, size: 64)
!56 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!57 = !DILocalVariable(name: "argc", arg: 1, scope: !51, file: !1, line: 25, type: !11)
!58 = !DILocation(line: 25, column: 14, scope: !51)
!59 = !DILocalVariable(name: "argv", arg: 2, scope: !51, file: !1, line: 25, type: !54)
!60 = !DILocation(line: 25, column: 27, scope: !51)
!61 = !DILocalVariable(name: "a", scope: !51, file: !1, line: 26, type: !62)
!62 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 64, elements: !63)
!63 = !{!64}
!64 = !DISubrange(count: 2)
!65 = !DILocation(line: 26, column: 7, scope: !51)
!66 = !DILocalVariable(name: "b", scope: !51, file: !1, line: 27, type: !67)
!67 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 96, elements: !68)
!68 = !{!69}
!69 = !DISubrange(count: 3)
!70 = !DILocation(line: 27, column: 7, scope: !51)
!71 = !DILocalVariable(name: "c", scope: !51, file: !1, line: 28, type: !72)
!72 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 320, elements: !73)
!73 = !{!74}
!74 = !DISubrange(count: 10)
!75 = !DILocation(line: 28, column: 7, scope: !51)
!76 = !DILocation(line: 30, column: 3, scope: !51)
!77 = !DILocation(line: 30, column: 8, scope: !51)
!78 = !DILocation(line: 32, column: 3, scope: !51)
!79 = !DILocation(line: 32, column: 8, scope: !51)
!80 = !DILocation(line: 33, column: 3, scope: !51)
!81 = !DILocation(line: 33, column: 8, scope: !51)
!82 = !DILocation(line: 35, column: 3, scope: !51)
!83 = !DILocation(line: 35, column: 8, scope: !51)
!84 = !DILocation(line: 36, column: 3, scope: !51)
!85 = !DILocation(line: 36, column: 8, scope: !51)
!86 = !DILocation(line: 37, column: 3, scope: !51)
!87 = !DILocation(line: 37, column: 8, scope: !51)
!88 = !DILocation(line: 39, column: 15, scope: !51)
!89 = !DILocation(line: 39, column: 18, scope: !51)
!90 = !DILocation(line: 39, column: 21, scope: !51)
!91 = !DILocation(line: 39, column: 11, scope: !51)
!92 = !DILocalVariable(name: "x", scope: !51, file: !1, line: 39, type: !11)
!93 = !DILocation(line: 39, column: 7, scope: !51)
!94 = !DILocation(line: 41, column: 19, scope: !51)
!95 = !DILocation(line: 41, column: 3, scope: !51)
!96 = !DILocation(line: 42, column: 17, scope: !51)
!97 = !DILocation(line: 42, column: 22, scope: !51)
!98 = !DILocation(line: 42, column: 3, scope: !51)
!99 = !DILocation(line: 43, column: 3, scope: !51)
