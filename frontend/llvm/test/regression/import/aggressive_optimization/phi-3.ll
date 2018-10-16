; ModuleID = 'phi-3.c.pp.bc'
source_filename = "phi-3.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

declare void @__ikos_assert(i32) local_unnamed_addr #2
; CHECK: declare void @ar.ikos.assert(ui32)

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc i32 @foo(i32*, i32*, i32*) unnamed_addr #0 !dbg !7 {
  call void @llvm.dbg.value(metadata i32* %0, i64 0, metadata !12, metadata !13), !dbg !14
  call void @llvm.dbg.value(metadata i32* %1, i64 0, metadata !15, metadata !13), !dbg !16
  call void @llvm.dbg.value(metadata i32* %2, i64 0, metadata !17, metadata !13), !dbg !18
  %4 = getelementptr inbounds i32, i32* %0, i64 1, !dbg !19
  call void @llvm.dbg.value(metadata i32* %4, i64 0, metadata !20, metadata !13), !dbg !21
  %5 = getelementptr inbounds i32, i32* %1, i64 2, !dbg !22
  call void @llvm.dbg.value(metadata i32* %5, i64 0, metadata !23, metadata !13), !dbg !24
  %6 = icmp eq i32* %4, %5, !dbg !25
  %7 = getelementptr inbounds i32, i32* %1, i64 -8, !dbg !27
  call void @llvm.dbg.value(metadata i32* %7, i64 0, metadata !23, metadata !13), !dbg !24
  %8 = getelementptr inbounds i32, i32* %0, i64 43, !dbg !29
  call void @llvm.dbg.value(metadata i32* %8, i64 0, metadata !20, metadata !13), !dbg !21
  br i1 %6, label %9, label %10, !dbg !30

; <label>:9:                                      ; preds = %3
  br label %11, !dbg !30

; <label>:10:                                     ; preds = %3
  br label %11, !dbg !30

; <label>:11:                                     ; preds = %10, %9
  %.01.phi = phi i32* [ %8, %9 ], [ %4, %10 ], !dbg !30
  br i1 %6, label %12, label %13, !dbg !30

; <label>:12:                                     ; preds = %11
  br label %14, !dbg !30

; <label>:13:                                     ; preds = %11
  br label %14, !dbg !30

; <label>:14:                                     ; preds = %13, %12
  %.0.phi = phi i32* [ %7, %12 ], [ %5, %13 ], !dbg !30
  call void @llvm.dbg.value(metadata i32* %.0.phi, i64 0, metadata !23, metadata !13), !dbg !24
  call void @llvm.dbg.value(metadata i32* %.01.phi, i64 0, metadata !20, metadata !13), !dbg !21
  %15 = load i32, i32* %.01.phi, align 4, !dbg !31
  %16 = icmp eq i32 %15, 3, !dbg !32
  %17 = zext i1 %16 to i32, !dbg !32
  call void @__ikos_assert(i32 %17) #3, !dbg !33
  %18 = load i32, i32* %.0.phi, align 4, !dbg !34
  %19 = icmp eq i32 %18, 6, !dbg !35
  %20 = zext i1 %19 to i32, !dbg !35
  call void @__ikos_assert(i32 %20) #3, !dbg !36
  %21 = load i32, i32* %.01.phi, align 4, !dbg !37
  %22 = load i32, i32* %.0.phi, align 4, !dbg !38
  %23 = add nsw i32 %21, %22, !dbg !39
  %24 = sext i32 %23 to i64, !dbg !40
  %25 = getelementptr inbounds i32, i32* %2, i64 %24, !dbg !40
  %26 = load i32, i32* %25, align 4, !dbg !40
  call void @llvm.dbg.value(metadata i32 %26, i64 0, metadata !41, metadata !13), !dbg !42
  store i32 555, i32* %25, align 4, !dbg !43
  ret i32 %26, !dbg !44
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
; CHECK:   si32* %7 = ptrshift %2, 4 * 18446744073709551608
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !45 {
  %3 = alloca [2 x i32], align 4
  %4 = alloca [3 x i32], align 4
  %5 = alloca [10 x i32], align 16
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !51, metadata !13), !dbg !52
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !53, metadata !13), !dbg !54
  call void @llvm.dbg.declare(metadata [2 x i32]* %3, metadata !55, metadata !13), !dbg !59
  call void @llvm.dbg.declare(metadata [3 x i32]* %4, metadata !60, metadata !13), !dbg !64
  call void @llvm.dbg.declare(metadata [10 x i32]* %5, metadata !65, metadata !13), !dbg !69
  %6 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i64 0, i64 9, !dbg !70
  store i32 666, i32* %6, align 4, !dbg !71
  %7 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 0, !dbg !72
  store i32 1, i32* %7, align 4, !dbg !73
  %8 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 1, !dbg !74
  store i32 3, i32* %8, align 4, !dbg !75
  %9 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 0, !dbg !76
  store i32 4, i32* %9, align 4, !dbg !77
  %10 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 1, !dbg !78
  store i32 5, i32* %10, align 4, !dbg !79
  %11 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 2, !dbg !80
  store i32 6, i32* %11, align 4, !dbg !81
  %12 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 0, !dbg !82
  %13 = getelementptr inbounds [3 x i32], [3 x i32]* %4, i64 0, i64 0, !dbg !83
  %14 = getelementptr inbounds [10 x i32], [10 x i32]* %5, i64 0, i64 0, !dbg !84
  %15 = call fastcc i32 @foo(i32* %12, i32* %13, i32* %14), !dbg !85
  call void @llvm.dbg.value(metadata i32 %15, i64 0, metadata !86, metadata !13), !dbg !87
  %16 = icmp eq i32 %15, 666, !dbg !88
  %17 = zext i1 %16 to i32, !dbg !88
  call void @__ikos_assert(i32 %17) #3, !dbg !89
  %18 = load i32, i32* %6, align 4, !dbg !90
  %19 = icmp eq i32 %18, 555, !dbg !91
  %20 = zext i1 %19 to i32, !dbg !91
  call void @__ikos_assert(i32 %20) #3, !dbg !92
  ret i32 %15, !dbg !93
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "phi-3.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 6, type: !8, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !11, !11, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!12 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 6, type: !11)
!13 = !DIExpression()
!14 = !DILocation(line: 6, column: 14, scope: !7)
!15 = !DILocalVariable(name: "b", arg: 2, scope: !7, file: !1, line: 6, type: !11)
!16 = !DILocation(line: 6, column: 22, scope: !7)
!17 = !DILocalVariable(name: "c", arg: 3, scope: !7, file: !1, line: 6, type: !11)
!18 = !DILocation(line: 6, column: 30, scope: !7)
!19 = !DILocation(line: 10, column: 9, scope: !7)
!20 = !DILocalVariable(name: "p", scope: !7, file: !1, line: 7, type: !11)
!21 = !DILocation(line: 7, column: 8, scope: !7)
!22 = !DILocation(line: 11, column: 9, scope: !7)
!23 = !DILocalVariable(name: "q", scope: !7, file: !1, line: 8, type: !11)
!24 = !DILocation(line: 8, column: 8, scope: !7)
!25 = !DILocation(line: 13, column: 9, scope: !26)
!26 = distinct !DILexicalBlock(scope: !7, file: !1, line: 13, column: 7)
!27 = !DILocation(line: 14, column: 11, scope: !28)
!28 = distinct !DILexicalBlock(scope: !26, file: !1, line: 13, column: 15)
!29 = !DILocation(line: 15, column: 11, scope: !28)
!30 = !DILocation(line: 13, column: 7, scope: !7)
!31 = !DILocation(line: 17, column: 17, scope: !7)
!32 = !DILocation(line: 17, column: 20, scope: !7)
!33 = !DILocation(line: 17, column: 3, scope: !7)
!34 = !DILocation(line: 18, column: 17, scope: !7)
!35 = !DILocation(line: 18, column: 20, scope: !7)
!36 = !DILocation(line: 18, column: 3, scope: !7)
!37 = !DILocation(line: 20, column: 15, scope: !7)
!38 = !DILocation(line: 20, column: 20, scope: !7)
!39 = !DILocation(line: 20, column: 18, scope: !7)
!40 = !DILocation(line: 20, column: 13, scope: !7)
!41 = !DILocalVariable(name: "res", scope: !7, file: !1, line: 20, type: !10)
!42 = !DILocation(line: 20, column: 7, scope: !7)
!43 = !DILocation(line: 21, column: 14, scope: !7)
!44 = !DILocation(line: 22, column: 3, scope: !7)
!45 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 25, type: !46, isLocal: false, isDefinition: true, scopeLine: 25, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!46 = !DISubroutineType(types: !47)
!47 = !{!10, !10, !48}
!48 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !49, size: 64)
!49 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !50, size: 64)
!50 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!51 = !DILocalVariable(name: "argc", arg: 1, scope: !45, file: !1, line: 25, type: !10)
!52 = !DILocation(line: 25, column: 14, scope: !45)
!53 = !DILocalVariable(name: "argv", arg: 2, scope: !45, file: !1, line: 25, type: !48)
!54 = !DILocation(line: 25, column: 27, scope: !45)
!55 = !DILocalVariable(name: "a", scope: !45, file: !1, line: 26, type: !56)
!56 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 64, elements: !57)
!57 = !{!58}
!58 = !DISubrange(count: 2)
!59 = !DILocation(line: 26, column: 7, scope: !45)
!60 = !DILocalVariable(name: "b", scope: !45, file: !1, line: 27, type: !61)
!61 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 96, elements: !62)
!62 = !{!63}
!63 = !DISubrange(count: 3)
!64 = !DILocation(line: 27, column: 7, scope: !45)
!65 = !DILocalVariable(name: "c", scope: !45, file: !1, line: 28, type: !66)
!66 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 320, elements: !67)
!67 = !{!68}
!68 = !DISubrange(count: 10)
!69 = !DILocation(line: 28, column: 7, scope: !45)
!70 = !DILocation(line: 30, column: 3, scope: !45)
!71 = !DILocation(line: 30, column: 8, scope: !45)
!72 = !DILocation(line: 32, column: 3, scope: !45)
!73 = !DILocation(line: 32, column: 8, scope: !45)
!74 = !DILocation(line: 33, column: 3, scope: !45)
!75 = !DILocation(line: 33, column: 8, scope: !45)
!76 = !DILocation(line: 35, column: 3, scope: !45)
!77 = !DILocation(line: 35, column: 8, scope: !45)
!78 = !DILocation(line: 36, column: 3, scope: !45)
!79 = !DILocation(line: 36, column: 8, scope: !45)
!80 = !DILocation(line: 37, column: 3, scope: !45)
!81 = !DILocation(line: 37, column: 8, scope: !45)
!82 = !DILocation(line: 39, column: 15, scope: !45)
!83 = !DILocation(line: 39, column: 18, scope: !45)
!84 = !DILocation(line: 39, column: 21, scope: !45)
!85 = !DILocation(line: 39, column: 11, scope: !45)
!86 = !DILocalVariable(name: "x", scope: !45, file: !1, line: 39, type: !10)
!87 = !DILocation(line: 39, column: 7, scope: !45)
!88 = !DILocation(line: 41, column: 19, scope: !45)
!89 = !DILocation(line: 41, column: 3, scope: !45)
!90 = !DILocation(line: 42, column: 17, scope: !45)
!91 = !DILocation(line: 42, column: 22, scope: !45)
!92 = !DILocation(line: 42, column: 3, scope: !45)
!93 = !DILocation(line: 43, column: 3, scope: !45)
