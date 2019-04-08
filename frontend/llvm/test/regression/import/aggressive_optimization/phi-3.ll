; ModuleID = 'phi-3.pp.bc'
source_filename = "phi-3.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

declare void @__ikos_assert(i32) local_unnamed_addr #2
; CHECK: declare void @ar.ikos.assert(ui32)

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc i32 @foo(i32*, i32*, i32*) unnamed_addr #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i32* %0, metadata !13, metadata !DIExpression()), !dbg !14
  call void @llvm.dbg.value(metadata i32* %1, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i32* %2, metadata !17, metadata !DIExpression()), !dbg !18
  %4 = getelementptr inbounds i32, i32* %0, i64 1, !dbg !19
  call void @llvm.dbg.value(metadata i32* %4, metadata !20, metadata !DIExpression()), !dbg !21
  %5 = getelementptr inbounds i32, i32* %1, i64 2, !dbg !22
  call void @llvm.dbg.value(metadata i32* %5, metadata !23, metadata !DIExpression()), !dbg !24
  %6 = icmp eq i32* %4, %5, !dbg !25
  %7 = getelementptr inbounds i32, i32* %1, i64 -8, !dbg !27
  %8 = getelementptr inbounds i32, i32* %0, i64 43, !dbg !27
  br i1 %6, label %9, label %10, !dbg !27

; <label>:9:                                      ; preds = %3
  br label %11, !dbg !27

; <label>:10:                                     ; preds = %3
  br label %11, !dbg !27

; <label>:11:                                     ; preds = %10, %9
  %.01.phi = phi i32* [ %8, %9 ], [ %4, %10 ], !dbg !27
  br i1 %6, label %12, label %13, !dbg !27

; <label>:12:                                     ; preds = %11
  br label %14, !dbg !27

; <label>:13:                                     ; preds = %11
  br label %14, !dbg !27

; <label>:14:                                     ; preds = %13, %12
  %.0.phi = phi i32* [ %7, %12 ], [ %5, %13 ], !dbg !27
  call void @llvm.dbg.value(metadata i32* %.0.phi, metadata !23, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.value(metadata i32* %.01.phi, metadata !20, metadata !DIExpression()), !dbg !21
  %15 = load i32, i32* %.01.phi, align 4, !dbg !28
  %16 = icmp eq i32 %15, 3, !dbg !29
  %17 = zext i1 %16 to i32, !dbg !29
  call void @__ikos_assert(i32 %17) #3, !dbg !30
  %18 = load i32, i32* %.0.phi, align 4, !dbg !31
  %19 = icmp eq i32 %18, 6, !dbg !32
  %20 = zext i1 %19 to i32, !dbg !32
  call void @__ikos_assert(i32 %20) #3, !dbg !33
  %21 = load i32, i32* %.01.phi, align 4, !dbg !34
  %22 = load i32, i32* %.0.phi, align 4, !dbg !35
  %23 = add nsw i32 %21, %22, !dbg !36
  %24 = sext i32 %23 to i64, !dbg !37
  %25 = getelementptr inbounds i32, i32* %2, i64 %24, !dbg !37
  %26 = load i32, i32* %25, align 4, !dbg !37
  call void @llvm.dbg.value(metadata i32 %26, metadata !38, metadata !DIExpression()), !dbg !39
  store i32 555, i32* %25, align 4, !dbg !40
  ret i32 %26, !dbg !41
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
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !42 {
  %3 = alloca [2 x i32], align 4
  %4 = alloca [3 x i32], align 4
  %5 = alloca [10 x i32], align 16
  call void @llvm.dbg.value(metadata i32 %0, metadata !48, metadata !DIExpression()), !dbg !49
  call void @llvm.dbg.value(metadata i8** %1, metadata !50, metadata !DIExpression()), !dbg !51
  call void @llvm.dbg.declare(metadata [2 x i32]* %3, metadata !52, metadata !DIExpression()), !dbg !56
  call void @llvm.dbg.declare(metadata [3 x i32]* %4, metadata !57, metadata !DIExpression()), !dbg !61
  call void @llvm.dbg.declare(metadata [10 x i32]* %5, metadata !62, metadata !DIExpression()), !dbg !66
  %6 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i64 0, i64 9, !dbg !67
  store i32 666, i32* %6, align 4, !dbg !68
  %7 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 0, !dbg !69
  store i32 1, i32* %7, align 4, !dbg !70
  %8 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 1, !dbg !71
  store i32 3, i32* %8, align 4, !dbg !72
  %9 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 0, !dbg !73
  store i32 4, i32* %9, align 4, !dbg !74
  %10 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 1, !dbg !75
  store i32 5, i32* %10, align 4, !dbg !76
  %11 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 2, !dbg !77
  store i32 6, i32* %11, align 4, !dbg !78
  %12 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 0, !dbg !79
  %13 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 0, !dbg !80
  %14 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i64 0, i64 0, !dbg !81
  %15 = call fastcc i32 @foo(i32* nonnull %12, i32* nonnull %13, i32* nonnull %14), !dbg !82
  call void @llvm.dbg.value(metadata i32 %15, metadata !83, metadata !DIExpression()), !dbg !84
  %16 = icmp eq i32 %15, 666, !dbg !85
  %17 = zext i1 %16 to i32, !dbg !85
  call void @__ikos_assert(i32 %17) #3, !dbg !86
  %18 = load i32, i32* %6, align 4, !dbg !87
  %19 = icmp eq i32 %18, 555, !dbg !88
  %20 = zext i1 %19 to i32, !dbg !88
  call void @__ikos_assert(i32 %20) #3, !dbg !89
  ret i32 %15, !dbg !90
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
; CHECK:   si32 %18 = load %6, align 4
; CHECK: }
; CHECK: #5 predecessors={#4} successors={#7} {
; CHECK:   %18 sieq 555
; CHECK:   ui1 %19 = 1
; CHECK: }
; CHECK: #6 predecessors={#4} successors={#7} {
; CHECK:   %18 sine 555
; CHECK:   ui1 %19 = 0
; CHECK: }
; CHECK: #7 !exit predecessors={#5, #6} {
; CHECK:   ui32 %20 = zext %19
; CHECK:   call @ar.ikos.assert(%20)
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
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 (tags/RELEASE_800/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "phi-3.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
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
!28 = !DILocation(line: 17, column: 17, scope: !8)
!29 = !DILocation(line: 17, column: 20, scope: !8)
!30 = !DILocation(line: 17, column: 3, scope: !8)
!31 = !DILocation(line: 18, column: 17, scope: !8)
!32 = !DILocation(line: 18, column: 20, scope: !8)
!33 = !DILocation(line: 18, column: 3, scope: !8)
!34 = !DILocation(line: 20, column: 15, scope: !8)
!35 = !DILocation(line: 20, column: 20, scope: !8)
!36 = !DILocation(line: 20, column: 18, scope: !8)
!37 = !DILocation(line: 20, column: 13, scope: !8)
!38 = !DILocalVariable(name: "res", scope: !8, file: !1, line: 20, type: !11)
!39 = !DILocation(line: 20, column: 7, scope: !8)
!40 = !DILocation(line: 21, column: 14, scope: !8)
!41 = !DILocation(line: 22, column: 3, scope: !8)
!42 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 25, type: !43, scopeLine: 25, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!43 = !DISubroutineType(types: !44)
!44 = !{!11, !11, !45}
!45 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !46, size: 64)
!46 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !47, size: 64)
!47 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!48 = !DILocalVariable(name: "argc", arg: 1, scope: !42, file: !1, line: 25, type: !11)
!49 = !DILocation(line: 25, column: 14, scope: !42)
!50 = !DILocalVariable(name: "argv", arg: 2, scope: !42, file: !1, line: 25, type: !45)
!51 = !DILocation(line: 25, column: 27, scope: !42)
!52 = !DILocalVariable(name: "a", scope: !42, file: !1, line: 26, type: !53)
!53 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 64, elements: !54)
!54 = !{!55}
!55 = !DISubrange(count: 2)
!56 = !DILocation(line: 26, column: 7, scope: !42)
!57 = !DILocalVariable(name: "b", scope: !42, file: !1, line: 27, type: !58)
!58 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 96, elements: !59)
!59 = !{!60}
!60 = !DISubrange(count: 3)
!61 = !DILocation(line: 27, column: 7, scope: !42)
!62 = !DILocalVariable(name: "c", scope: !42, file: !1, line: 28, type: !63)
!63 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 320, elements: !64)
!64 = !{!65}
!65 = !DISubrange(count: 10)
!66 = !DILocation(line: 28, column: 7, scope: !42)
!67 = !DILocation(line: 30, column: 3, scope: !42)
!68 = !DILocation(line: 30, column: 8, scope: !42)
!69 = !DILocation(line: 32, column: 3, scope: !42)
!70 = !DILocation(line: 32, column: 8, scope: !42)
!71 = !DILocation(line: 33, column: 3, scope: !42)
!72 = !DILocation(line: 33, column: 8, scope: !42)
!73 = !DILocation(line: 35, column: 3, scope: !42)
!74 = !DILocation(line: 35, column: 8, scope: !42)
!75 = !DILocation(line: 36, column: 3, scope: !42)
!76 = !DILocation(line: 36, column: 8, scope: !42)
!77 = !DILocation(line: 37, column: 3, scope: !42)
!78 = !DILocation(line: 37, column: 8, scope: !42)
!79 = !DILocation(line: 39, column: 15, scope: !42)
!80 = !DILocation(line: 39, column: 18, scope: !42)
!81 = !DILocation(line: 39, column: 21, scope: !42)
!82 = !DILocation(line: 39, column: 11, scope: !42)
!83 = !DILocalVariable(name: "x", scope: !42, file: !1, line: 39, type: !11)
!84 = !DILocation(line: 39, column: 7, scope: !42)
!85 = !DILocation(line: 41, column: 19, scope: !42)
!86 = !DILocation(line: 41, column: 3, scope: !42)
!87 = !DILocation(line: 42, column: 17, scope: !42)
!88 = !DILocation(line: 42, column: 22, scope: !42)
!89 = !DILocation(line: 42, column: 3, scope: !42)
!90 = !DILocation(line: 43, column: 3, scope: !42)
