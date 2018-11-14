; ModuleID = 'phi-2.c.pp.bc'
source_filename = "phi-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

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
  call void @llvm.dbg.value(metadata i8** %1, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.declare(metadata %struct.foo* %3, metadata !19, metadata !DIExpression()), !dbg !34
  call void @llvm.dbg.value(metadata i32 0, metadata !35, metadata !DIExpression()), !dbg !36
  br label %4, !dbg !37

; <label>:4:                                      ; preds = %36, %2
  %.02 = phi i32 [ 0, %2 ], [ %37, %36 ]
  call void @llvm.dbg.value(metadata i32 %.02, metadata !35, metadata !DIExpression()), !dbg !36
  %5 = icmp slt i32 %.02, 10, !dbg !39
  br i1 %5, label %6, label %38, !dbg !41

; <label>:6:                                      ; preds = %4
  call void @llvm.dbg.value(metadata i32 0, metadata !42, metadata !DIExpression()), !dbg !43
  br label %7, !dbg !44

; <label>:7:                                      ; preds = %33, %6
  %.01 = phi i32 [ 0, %6 ], [ %34, %33 ]
  call void @llvm.dbg.value(metadata i32 %.01, metadata !42, metadata !DIExpression()), !dbg !43
  %8 = icmp slt i32 %.01, 10, !dbg !47
  br i1 %8, label %9, label %35, !dbg !49

; <label>:9:                                      ; preds = %7
  call void @llvm.dbg.value(metadata i32 0, metadata !50, metadata !DIExpression()), !dbg !51
  br label %10, !dbg !52

; <label>:10:                                     ; preds = %30, %9
  %.0 = phi i32 [ 0, %9 ], [ %31, %30 ]
  call void @llvm.dbg.value(metadata i32 %.0, metadata !50, metadata !DIExpression()), !dbg !51
  %11 = icmp slt i32 %.0, 9, !dbg !55
  br i1 %11, label %12, label %32, !dbg !57

; <label>:12:                                     ; preds = %10
  %13 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 2, !dbg !58
  %14 = sext i32 %.02 to i64, !dbg !60
  %15 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %13, i64 0, i64 %14, !dbg !60
  %16 = sext i32 %.01 to i64, !dbg !60
  %17 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %15, i64 0, i64 %16, !dbg !60
  %18 = sext i32 %.0 to i64, !dbg !60
  %19 = getelementptr inbounds [9 x i32], [9 x i32]* %17, i64 0, i64 %18, !dbg !60
  store i32 %0, i32* %19, align 4, !dbg !61
  %20 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 2, !dbg !62
  %21 = sext i32 %.02 to i64, !dbg !63
  %22 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %20, i64 0, i64 %21, !dbg !63
  %23 = sext i32 %.01 to i64, !dbg !63
  %24 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %22, i64 0, i64 %23, !dbg !63
  %25 = sext i32 %.0 to i64, !dbg !63
  %26 = getelementptr inbounds [9 x i32], [9 x i32]* %24, i64 0, i64 %25, !dbg !63
  %27 = load i32, i32* %26, align 4, !dbg !63
  %28 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 1, !dbg !64
  %29 = getelementptr inbounds %struct.bar, %struct.bar* %28, i32 0, i32 0, !dbg !65
  store i32 %27, i32* %29, align 4, !dbg !66
  br label %30, !dbg !67

; <label>:30:                                     ; preds = %12
  %31 = add nsw i32 %.0, 1, !dbg !68
  call void @llvm.dbg.value(metadata i32 %31, metadata !50, metadata !DIExpression()), !dbg !51
  br label %10, !dbg !69, !llvm.loop !70

; <label>:32:                                     ; preds = %10
  br label %33, !dbg !72

; <label>:33:                                     ; preds = %32
  %34 = add nsw i32 %.01, 1, !dbg !73
  call void @llvm.dbg.value(metadata i32 %34, metadata !42, metadata !DIExpression()), !dbg !43
  br label %7, !dbg !74, !llvm.loop !75

; <label>:35:                                     ; preds = %7
  br label %36, !dbg !77

; <label>:36:                                     ; preds = %35
  %37 = add nsw i32 %.02, 1, !dbg !78
  call void @llvm.dbg.value(metadata i32 %37, metadata !35, metadata !DIExpression()), !dbg !36
  br label %4, !dbg !79, !llvm.loop !80

; <label>:38:                                     ; preds = %4
  call void @llvm.dbg.value(metadata i32 0, metadata !35, metadata !DIExpression()), !dbg !36
  br label %39, !dbg !82

; <label>:39:                                     ; preds = %53, %38
  %.1 = phi i32 [ 0, %38 ], [ %54, %53 ]
  call void @llvm.dbg.value(metadata i32 %.1, metadata !35, metadata !DIExpression()), !dbg !36
  %40 = icmp slt i32 %.1, 10, !dbg !84
  br i1 %40, label %41, label %55, !dbg !86

; <label>:41:                                     ; preds = %39
  %42 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 2, !dbg !87
  %43 = sext i32 %.1 to i64, !dbg !89
  %44 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %42, i64 0, i64 %43, !dbg !89
  %45 = sext i32 %.1 to i64, !dbg !89
  %46 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %44, i64 0, i64 %45, !dbg !89
  %47 = sub nsw i32 %.1, 1, !dbg !90
  %48 = sext i32 %47 to i64, !dbg !89
  %49 = getelementptr inbounds [9 x i32], [9 x i32]* %46, i64 0, i64 %48, !dbg !89
  %50 = load i32, i32* %49, align 4, !dbg !89
  %51 = getelementptr inbounds [4 x i8], [4 x i8]* @.str, i32 0, i32 0, !dbg !91
  %52 = call i32 (i8*, ...) @printf(i8* %51, i32 %50), !dbg !91
  br label %53, !dbg !92

; <label>:53:                                     ; preds = %41
  %54 = add nsw i32 %.1, 1, !dbg !93
  call void @llvm.dbg.value(metadata i32 %54, metadata !35, metadata !DIExpression()), !dbg !36
  br label %39, !dbg !94, !llvm.loop !95

; <label>:55:                                     ; preds = %39
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

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 6.0.1 (tags/RELEASE_601/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "phi-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 6.0.1 (tags/RELEASE_601/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 18, type: !9, isLocal: false, isDefinition: true, scopeLine: 18, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!15 = !DILocalVariable(name: "argc", arg: 1, scope: !8, file: !1, line: 18, type: !11)
!16 = !DILocation(line: 18, column: 14, scope: !8)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !8, file: !1, line: 18, type: !12)
!18 = !DILocation(line: 18, column: 27, scope: !8)
!19 = !DILocalVariable(name: "x", scope: !8, file: !1, line: 20, type: !20)
!20 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "foo", file: !1, line: 11, size: 28896, elements: !21)
!21 = !{!22, !23, !29}
!22 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !20, file: !1, line: 12, baseType: !11, size: 32)
!23 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !20, file: !1, line: 13, baseType: !24, size: 64, offset: 32)
!24 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bar", file: !1, line: 6, size: 64, elements: !25)
!25 = !{!26, !27}
!26 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !24, file: !1, line: 7, baseType: !11, size: 32)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !24, file: !1, line: 8, baseType: !28, size: 32, offset: 32)
!28 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!29 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !20, file: !1, line: 14, baseType: !30, size: 28800, offset: 96)
!30 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 28800, elements: !31)
!31 = !{!32, !32, !33}
!32 = !DISubrange(count: 10)
!33 = !DISubrange(count: 9)
!34 = !DILocation(line: 20, column: 14, scope: !8)
!35 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 19, type: !11)
!36 = !DILocation(line: 19, column: 7, scope: !8)
!37 = !DILocation(line: 21, column: 8, scope: !38)
!38 = distinct !DILexicalBlock(scope: !8, file: !1, line: 21, column: 3)
!39 = !DILocation(line: 21, column: 17, scope: !40)
!40 = distinct !DILexicalBlock(scope: !38, file: !1, line: 21, column: 3)
!41 = !DILocation(line: 21, column: 3, scope: !38)
!42 = !DILocalVariable(name: "j", scope: !8, file: !1, line: 19, type: !11)
!43 = !DILocation(line: 19, column: 10, scope: !8)
!44 = !DILocation(line: 22, column: 10, scope: !45)
!45 = distinct !DILexicalBlock(scope: !46, file: !1, line: 22, column: 5)
!46 = distinct !DILexicalBlock(scope: !40, file: !1, line: 21, column: 35)
!47 = !DILocation(line: 22, column: 19, scope: !48)
!48 = distinct !DILexicalBlock(scope: !45, file: !1, line: 22, column: 5)
!49 = !DILocation(line: 22, column: 5, scope: !45)
!50 = !DILocalVariable(name: "k", scope: !8, file: !1, line: 19, type: !11)
!51 = !DILocation(line: 19, column: 13, scope: !8)
!52 = !DILocation(line: 23, column: 12, scope: !53)
!53 = distinct !DILexicalBlock(scope: !54, file: !1, line: 23, column: 7)
!54 = distinct !DILexicalBlock(scope: !48, file: !1, line: 22, column: 37)
!55 = !DILocation(line: 23, column: 21, scope: !56)
!56 = distinct !DILexicalBlock(scope: !53, file: !1, line: 23, column: 7)
!57 = !DILocation(line: 23, column: 7, scope: !53)
!58 = !DILocation(line: 24, column: 11, scope: !59)
!59 = distinct !DILexicalBlock(scope: !56, file: !1, line: 23, column: 43)
!60 = !DILocation(line: 24, column: 9, scope: !59)
!61 = !DILocation(line: 24, column: 22, scope: !59)
!62 = !DILocation(line: 25, column: 19, scope: !59)
!63 = !DILocation(line: 25, column: 17, scope: !59)
!64 = !DILocation(line: 25, column: 11, scope: !59)
!65 = !DILocation(line: 25, column: 13, scope: !59)
!66 = !DILocation(line: 25, column: 15, scope: !59)
!67 = !DILocation(line: 26, column: 7, scope: !59)
!68 = !DILocation(line: 23, column: 39, scope: !56)
!69 = !DILocation(line: 23, column: 7, scope: !56)
!70 = distinct !{!70, !57, !71}
!71 = !DILocation(line: 26, column: 7, scope: !53)
!72 = !DILocation(line: 27, column: 5, scope: !54)
!73 = !DILocation(line: 22, column: 33, scope: !48)
!74 = !DILocation(line: 22, column: 5, scope: !48)
!75 = distinct !{!75, !49, !76}
!76 = !DILocation(line: 27, column: 5, scope: !45)
!77 = !DILocation(line: 28, column: 3, scope: !46)
!78 = !DILocation(line: 21, column: 31, scope: !40)
!79 = !DILocation(line: 21, column: 3, scope: !40)
!80 = distinct !{!80, !41, !81}
!81 = !DILocation(line: 28, column: 3, scope: !38)
!82 = !DILocation(line: 30, column: 8, scope: !83)
!83 = distinct !DILexicalBlock(scope: !8, file: !1, line: 30, column: 3)
!84 = !DILocation(line: 30, column: 17, scope: !85)
!85 = distinct !DILexicalBlock(scope: !83, file: !1, line: 30, column: 3)
!86 = !DILocation(line: 30, column: 3, scope: !83)
!87 = !DILocation(line: 31, column: 22, scope: !88)
!88 = distinct !DILexicalBlock(scope: !85, file: !1, line: 30, column: 35)
!89 = !DILocation(line: 31, column: 20, scope: !88)
!90 = !DILocation(line: 31, column: 32, scope: !88)
!91 = !DILocation(line: 31, column: 5, scope: !88)
!92 = !DILocation(line: 32, column: 3, scope: !88)
!93 = !DILocation(line: 30, column: 31, scope: !85)
!94 = !DILocation(line: 30, column: 3, scope: !85)
!95 = distinct !{!95, !86, !96}
!96 = !DILocation(line: 32, column: 3, scope: !83)
!97 = !DILocation(line: 34, column: 3, scope: !8)
