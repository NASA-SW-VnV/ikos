; ModuleID = 'phi-3.pp.bc'
source_filename = "phi-3.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

declare void @__ikos_assert(i32) local_unnamed_addr #2
; CHECK: declare void @ar.ikos.assert(ui32)

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc i32 @foo(i32*, i32*, i32*) unnamed_addr #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i32* %0, metadata !13, metadata !DIExpression()), !dbg !14
  call void @llvm.dbg.value(metadata i32* %1, metadata !15, metadata !DIExpression()), !dbg !14
  call void @llvm.dbg.value(metadata i32* %2, metadata !16, metadata !DIExpression()), !dbg !14
  %4 = getelementptr inbounds i32, i32* %0, i64 1, !dbg !17
  call void @llvm.dbg.value(metadata i32* %4, metadata !18, metadata !DIExpression()), !dbg !14
  %5 = getelementptr inbounds i32, i32* %1, i64 2, !dbg !19
  call void @llvm.dbg.value(metadata i32* %5, metadata !20, metadata !DIExpression()), !dbg !14
  %6 = icmp eq i32* %4, %5, !dbg !21
  %7 = getelementptr inbounds i32, i32* %1, i64 -8, !dbg !23
  %8 = getelementptr inbounds i32, i32* %0, i64 43, !dbg !23
  br i1 %6, label %9, label %10, !dbg !23

9:                                                ; preds = %3
  br label %11, !dbg !23

10:                                               ; preds = %3
  br label %11, !dbg !23

11:                                               ; preds = %10, %9
  %.01.phi = phi i32* [ %8, %9 ], [ %4, %10 ], !dbg !23
  br i1 %6, label %12, label %13, !dbg !23

12:                                               ; preds = %11
  br label %14, !dbg !23

13:                                               ; preds = %11
  br label %14, !dbg !23

14:                                               ; preds = %13, %12
  %.0.phi = phi i32* [ %7, %12 ], [ %5, %13 ], !dbg !23
  call void @llvm.dbg.value(metadata i32* %.0.phi, metadata !20, metadata !DIExpression()), !dbg !14
  call void @llvm.dbg.value(metadata i32* %.01.phi, metadata !18, metadata !DIExpression()), !dbg !14
  %15 = load i32, i32* %.01.phi, align 4, !dbg !24
  %16 = icmp eq i32 %15, 3, !dbg !25
  %17 = zext i1 %16 to i32, !dbg !25
  call void @__ikos_assert(i32 %17) #3, !dbg !26
  %18 = load i32, i32* %.0.phi, align 4, !dbg !27
  %19 = icmp eq i32 %18, 6, !dbg !28
  %20 = zext i1 %19 to i32, !dbg !28
  call void @__ikos_assert(i32 %20) #3, !dbg !29
  %21 = load i32, i32* %.01.phi, align 4, !dbg !30
  %22 = load i32, i32* %.0.phi, align 4, !dbg !31
  %23 = add nsw i32 %21, %22, !dbg !32
  %24 = sext i32 %23 to i64, !dbg !33
  %25 = getelementptr inbounds i32, i32* %2, i64 %24, !dbg !33
  %26 = load i32, i32* %25, align 4, !dbg !33
  call void @llvm.dbg.value(metadata i32 %26, metadata !34, metadata !DIExpression()), !dbg !14
  store i32 555, i32* %25, align 4, !dbg !35
  ret i32 %26, !dbg !36
}
; CHECK: define si32 @foo(si32* %1, si32* %2, si32* %3) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32* %4 = ptrshift %1, 4 * 1
; CHECK:   si32* %5 = ptrshift %2, 4 * 2
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   %4 peq %5
; CHECK:   ui1 %6 = 1
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   %4 pne %5
; CHECK:   ui1 %6 = 0
; CHECK: }
; CHECK: #4 predecessors={#2, #3} successors={#5, #6} {
; CHECK:   si32* %7 = ptrshift %2, 4 * -8
; CHECK:   si32* %8 = ptrshift %1, 4 * 43
; CHECK: }
; CHECK: #5 predecessors={#4} successors={#7} {
; CHECK:   %6 uieq 1
; CHECK:   si32* %.01.phi = %8
; CHECK: }
; CHECK: #6 predecessors={#4} successors={#7} {
; CHECK:   %6 uieq 0
; CHECK:   si32* %.01.phi = %4
; CHECK: }
; CHECK: #7 predecessors={#5, #6} successors={#8, #9} {
; CHECK: }
; CHECK: #8 predecessors={#7} successors={#10} {
; CHECK:   %6 uieq 1
; CHECK:   si32* %.0.phi = %7
; CHECK: }
; CHECK: #9 predecessors={#7} successors={#10} {
; CHECK:   %6 uieq 0
; CHECK:   si32* %.0.phi = %5
; CHECK: }
; CHECK: #10 predecessors={#8, #9} successors={#11, #12} {
; CHECK:   si32 %9 = load %.01.phi, align 4
; CHECK: }
; CHECK: #11 predecessors={#10} successors={#13} {
; CHECK:   %9 sieq 3
; CHECK:   ui1 %10 = 1
; CHECK: }
; CHECK: #12 predecessors={#10} successors={#13} {
; CHECK:   %9 sine 3
; CHECK:   ui1 %10 = 0
; CHECK: }
; CHECK: #13 predecessors={#11, #12} successors={#14, #15} {
; CHECK:   ui32 %11 = zext %10
; CHECK:   call @ar.ikos.assert(%11)
; CHECK:   si32 %12 = load %.0.phi, align 4
; CHECK: }
; CHECK: #14 predecessors={#13} successors={#16} {
; CHECK:   %12 sieq 6
; CHECK:   ui1 %13 = 1
; CHECK: }
; CHECK: #15 predecessors={#13} successors={#16} {
; CHECK:   %12 sine 6
; CHECK:   ui1 %13 = 0
; CHECK: }
; CHECK: #16 !exit predecessors={#14, #15} {
; CHECK:   ui32 %14 = zext %13
; CHECK:   call @ar.ikos.assert(%14)
; CHECK:   si32 %15 = load %.01.phi, align 4
; CHECK:   si32 %16 = load %.0.phi, align 4
; CHECK:   si32 %17 = %15 sadd.nw %16
; CHECK:   si64 %18 = sext %17
; CHECK:   si32* %19 = ptrshift %3, 4 * %18
; CHECK:   si32 %20 = load %19, align 4
; CHECK:   store %19, 555, align 4
; CHECK:   return %20
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !37 {
  %3 = alloca [2 x i32], align 4
  %4 = alloca [3 x i32], align 4
  %5 = alloca [10 x i32], align 16
  call void @llvm.dbg.value(metadata i32 %0, metadata !43, metadata !DIExpression()), !dbg !44
  call void @llvm.dbg.value(metadata i8** %1, metadata !45, metadata !DIExpression()), !dbg !44
  call void @llvm.dbg.declare(metadata [2 x i32]* %3, metadata !46, metadata !DIExpression()), !dbg !50
  call void @llvm.dbg.declare(metadata [3 x i32]* %4, metadata !51, metadata !DIExpression()), !dbg !55
  call void @llvm.dbg.declare(metadata [10 x i32]* %5, metadata !56, metadata !DIExpression()), !dbg !60
  %6 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i64 0, i64 9, !dbg !61
  store i32 666, i32* %6, align 4, !dbg !62
  %7 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 0, !dbg !63
  store i32 1, i32* %7, align 4, !dbg !64
  %8 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 1, !dbg !65
  store i32 3, i32* %8, align 4, !dbg !66
  %9 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 0, !dbg !67
  store i32 4, i32* %9, align 4, !dbg !68
  %10 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 1, !dbg !69
  store i32 5, i32* %10, align 4, !dbg !70
  %11 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 2, !dbg !71
  store i32 6, i32* %11, align 4, !dbg !72
  %12 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i64 0, i64 0, !dbg !73
  %13 = call fastcc i32 @foo(i32* nonnull %7, i32* nonnull %9, i32* nonnull %12), !dbg !74
  call void @llvm.dbg.value(metadata i32 %13, metadata !75, metadata !DIExpression()), !dbg !44
  %14 = icmp eq i32 %13, 666, !dbg !76
  %15 = zext i1 %14 to i32, !dbg !76
  call void @__ikos_assert(i32 %15) #3, !dbg !77
  %16 = load i32, i32* %6, align 4, !dbg !78
  %17 = icmp eq i32 %16, 555, !dbg !79
  %18 = zext i1 %17 to i32, !dbg !79
  call void @__ikos_assert(i32 %18) #3, !dbg !80
  ret i32 %13, !dbg !81
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
; CHECK:   si32* %12 = ptrshift $5, 40 * 0, 4 * 0
; CHECK:   si32 %13 = call @foo(%7, %9, %12)
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   %13 sieq 666
; CHECK:   ui1 %14 = 1
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   %13 sine 666
; CHECK:   ui1 %14 = 0
; CHECK: }
; CHECK: #4 predecessors={#2, #3} successors={#5, #6} {
; CHECK:   ui32 %15 = zext %14
; CHECK:   call @ar.ikos.assert(%15)
; CHECK:   si32 %16 = load %6, align 4
; CHECK: }
; CHECK: #5 predecessors={#4} successors={#7} {
; CHECK:   %16 sieq 555
; CHECK:   ui1 %17 = 1
; CHECK: }
; CHECK: #6 predecessors={#4} successors={#7} {
; CHECK:   %16 sine 555
; CHECK:   ui1 %17 = 0
; CHECK: }
; CHECK: #7 !exit predecessors={#5, #6} {
; CHECK:   ui32 %18 = zext %17
; CHECK:   call @ar.ikos.assert(%18)
; CHECK:   return %13
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "phi-3.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 5, type: !9, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !12, !12, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!13 = !DILocalVariable(name: "a", arg: 1, scope: !8, file: !1, line: 5, type: !12)
!14 = !DILocation(line: 0, scope: !8)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !8, file: !1, line: 5, type: !12)
!16 = !DILocalVariable(name: "c", arg: 3, scope: !8, file: !1, line: 5, type: !12)
!17 = !DILocation(line: 9, column: 9, scope: !8)
!18 = !DILocalVariable(name: "p", scope: !8, file: !1, line: 6, type: !12)
!19 = !DILocation(line: 10, column: 9, scope: !8)
!20 = !DILocalVariable(name: "q", scope: !8, file: !1, line: 7, type: !12)
!21 = !DILocation(line: 12, column: 9, scope: !22)
!22 = distinct !DILexicalBlock(scope: !8, file: !1, line: 12, column: 7)
!23 = !DILocation(line: 12, column: 7, scope: !8)
!24 = !DILocation(line: 16, column: 17, scope: !8)
!25 = !DILocation(line: 16, column: 20, scope: !8)
!26 = !DILocation(line: 16, column: 3, scope: !8)
!27 = !DILocation(line: 17, column: 17, scope: !8)
!28 = !DILocation(line: 17, column: 20, scope: !8)
!29 = !DILocation(line: 17, column: 3, scope: !8)
!30 = !DILocation(line: 19, column: 15, scope: !8)
!31 = !DILocation(line: 19, column: 20, scope: !8)
!32 = !DILocation(line: 19, column: 18, scope: !8)
!33 = !DILocation(line: 19, column: 13, scope: !8)
!34 = !DILocalVariable(name: "res", scope: !8, file: !1, line: 19, type: !11)
!35 = !DILocation(line: 20, column: 14, scope: !8)
!36 = !DILocation(line: 21, column: 3, scope: !8)
!37 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 24, type: !38, scopeLine: 24, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!38 = !DISubroutineType(types: !39)
!39 = !{!11, !11, !40}
!40 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !41, size: 64)
!41 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !42, size: 64)
!42 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!43 = !DILocalVariable(name: "argc", arg: 1, scope: !37, file: !1, line: 24, type: !11)
!44 = !DILocation(line: 0, scope: !37)
!45 = !DILocalVariable(name: "argv", arg: 2, scope: !37, file: !1, line: 24, type: !40)
!46 = !DILocalVariable(name: "a", scope: !37, file: !1, line: 25, type: !47)
!47 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 64, elements: !48)
!48 = !{!49}
!49 = !DISubrange(count: 2)
!50 = !DILocation(line: 25, column: 7, scope: !37)
!51 = !DILocalVariable(name: "b", scope: !37, file: !1, line: 26, type: !52)
!52 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 96, elements: !53)
!53 = !{!54}
!54 = !DISubrange(count: 3)
!55 = !DILocation(line: 26, column: 7, scope: !37)
!56 = !DILocalVariable(name: "c", scope: !37, file: !1, line: 27, type: !57)
!57 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 320, elements: !58)
!58 = !{!59}
!59 = !DISubrange(count: 10)
!60 = !DILocation(line: 27, column: 7, scope: !37)
!61 = !DILocation(line: 29, column: 3, scope: !37)
!62 = !DILocation(line: 29, column: 8, scope: !37)
!63 = !DILocation(line: 31, column: 3, scope: !37)
!64 = !DILocation(line: 31, column: 8, scope: !37)
!65 = !DILocation(line: 32, column: 3, scope: !37)
!66 = !DILocation(line: 32, column: 8, scope: !37)
!67 = !DILocation(line: 34, column: 3, scope: !37)
!68 = !DILocation(line: 34, column: 8, scope: !37)
!69 = !DILocation(line: 35, column: 3, scope: !37)
!70 = !DILocation(line: 35, column: 8, scope: !37)
!71 = !DILocation(line: 36, column: 3, scope: !37)
!72 = !DILocation(line: 36, column: 8, scope: !37)
!73 = !DILocation(line: 38, column: 21, scope: !37)
!74 = !DILocation(line: 38, column: 11, scope: !37)
!75 = !DILocalVariable(name: "x", scope: !37, file: !1, line: 38, type: !11)
!76 = !DILocation(line: 40, column: 19, scope: !37)
!77 = !DILocation(line: 40, column: 3, scope: !37)
!78 = !DILocation(line: 41, column: 17, scope: !37)
!79 = !DILocation(line: 41, column: 22, scope: !37)
!80 = !DILocation(line: 41, column: 3, scope: !37)
!81 = !DILocation(line: 42, column: 3, scope: !37)
