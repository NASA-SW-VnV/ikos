; ModuleID = 'phi-2.c.pp.bc'
source_filename = "phi-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

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
define i32 @main(i32, i8**) #0 !dbg !7 {
  %3 = alloca %struct.foo, align 4
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !14, metadata !15), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !17, metadata !15), !dbg !18
  call void @llvm.dbg.declare(metadata %struct.foo* %3, metadata !19, metadata !15), !dbg !34
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !35, metadata !15), !dbg !36
  br label %4, !dbg !37

; <label>:4:                                      ; preds = %36, %2
  %.02 = phi i32 [ 0, %2 ], [ %37, %36 ]
  call void @llvm.dbg.value(metadata i32 %.02, i64 0, metadata !35, metadata !15), !dbg !36
  %5 = icmp slt i32 %.02, 10, !dbg !39
  br i1 %5, label %6, label %38, !dbg !42

; <label>:6:                                      ; preds = %4
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !44, metadata !15), !dbg !45
  br label %7, !dbg !46

; <label>:7:                                      ; preds = %33, %6
  %.01 = phi i32 [ 0, %6 ], [ %34, %33 ]
  call void @llvm.dbg.value(metadata i32 %.01, i64 0, metadata !44, metadata !15), !dbg !45
  %8 = icmp slt i32 %.01, 10, !dbg !49
  br i1 %8, label %9, label %35, !dbg !52

; <label>:9:                                      ; preds = %7
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !54, metadata !15), !dbg !55
  br label %10, !dbg !56

; <label>:10:                                     ; preds = %30, %9
  %.0 = phi i32 [ 0, %9 ], [ %31, %30 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !54, metadata !15), !dbg !55
  %11 = icmp slt i32 %.0, 9, !dbg !59
  br i1 %11, label %12, label %32, !dbg !62

; <label>:12:                                     ; preds = %10
  %13 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 2, !dbg !64
  %14 = sext i32 %.02 to i64, !dbg !66
  %15 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %13, i64 0, i64 %14, !dbg !66
  %16 = sext i32 %.01 to i64, !dbg !66
  %17 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %15, i64 0, i64 %16, !dbg !66
  %18 = sext i32 %.0 to i64, !dbg !66
  %19 = getelementptr inbounds [9 x i32], [9 x i32]* %17, i64 0, i64 %18, !dbg !66
  store i32 %0, i32* %19, align 4, !dbg !67
  %20 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 2, !dbg !68
  %21 = sext i32 %.02 to i64, !dbg !69
  %22 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %20, i64 0, i64 %21, !dbg !69
  %23 = sext i32 %.01 to i64, !dbg !69
  %24 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %22, i64 0, i64 %23, !dbg !69
  %25 = sext i32 %.0 to i64, !dbg !69
  %26 = getelementptr inbounds [9 x i32], [9 x i32]* %24, i64 0, i64 %25, !dbg !69
  %27 = load i32, i32* %26, align 4, !dbg !69
  %28 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 1, !dbg !70
  %29 = getelementptr inbounds %struct.bar, %struct.bar* %28, i32 0, i32 0, !dbg !71
  store i32 %27, i32* %29, align 4, !dbg !72
  br label %30, !dbg !73

; <label>:30:                                     ; preds = %12
  %31 = add nsw i32 %.0, 1, !dbg !74
  call void @llvm.dbg.value(metadata i32 %31, i64 0, metadata !54, metadata !15), !dbg !55
  br label %10, !dbg !76, !llvm.loop !77

; <label>:32:                                     ; preds = %10
  br label %33, !dbg !80

; <label>:33:                                     ; preds = %32
  %34 = add nsw i32 %.01, 1, !dbg !81
  call void @llvm.dbg.value(metadata i32 %34, i64 0, metadata !44, metadata !15), !dbg !45
  br label %7, !dbg !83, !llvm.loop !84

; <label>:35:                                     ; preds = %7
  br label %36, !dbg !87

; <label>:36:                                     ; preds = %35
  %37 = add nsw i32 %.02, 1, !dbg !88
  call void @llvm.dbg.value(metadata i32 %37, i64 0, metadata !35, metadata !15), !dbg !36
  br label %4, !dbg !90, !llvm.loop !91

; <label>:38:                                     ; preds = %4
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !35, metadata !15), !dbg !36
  br label %39, !dbg !94

; <label>:39:                                     ; preds = %53, %38
  %.1 = phi i32 [ 0, %38 ], [ %54, %53 ]
  call void @llvm.dbg.value(metadata i32 %.1, i64 0, metadata !35, metadata !15), !dbg !36
  %40 = icmp slt i32 %.1, 10, !dbg !96
  br i1 %40, label %41, label %55, !dbg !99

; <label>:41:                                     ; preds = %39
  %42 = getelementptr inbounds %struct.foo, %struct.foo* %3, i32 0, i32 2, !dbg !101
  %43 = sext i32 %.1 to i64, !dbg !103
  %44 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %42, i64 0, i64 %43, !dbg !103
  %45 = sext i32 %.1 to i64, !dbg !103
  %46 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %44, i64 0, i64 %45, !dbg !103
  %47 = sub nsw i32 %.1, 1, !dbg !104
  %48 = sext i32 %47 to i64, !dbg !103
  %49 = getelementptr inbounds [9 x i32], [9 x i32]* %46, i64 0, i64 %48, !dbg !103
  %50 = load i32, i32* %49, align 4, !dbg !103
  %51 = getelementptr inbounds [4 x i8], [4 x i8]* @.str, i32 0, i32 0, !dbg !105
  %52 = call i32 (i8*, ...) @printf(i8* %51, i32 %50), !dbg !105
  br label %53, !dbg !106

; <label>:53:                                     ; preds = %41
  %54 = add nsw i32 %.1, 1, !dbg !107
  call void @llvm.dbg.value(metadata i32 %54, i64 0, metadata !35, metadata !15), !dbg !36
  br label %39, !dbg !109, !llvm.loop !110

; <label>:55:                                     ; preds = %39
  ret i32 0, !dbg !113
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "phi-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 18, type: !8, isLocal: false, isDefinition: true, scopeLine: 18, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!14 = !DILocalVariable(name: "argc", arg: 1, scope: !7, file: !1, line: 18, type: !10)
!15 = !DIExpression()
!16 = !DILocation(line: 18, column: 14, scope: !7)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !7, file: !1, line: 18, type: !11)
!18 = !DILocation(line: 18, column: 27, scope: !7)
!19 = !DILocalVariable(name: "x", scope: !7, file: !1, line: 20, type: !20)
!20 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "foo", file: !1, line: 11, size: 28896, elements: !21)
!21 = !{!22, !23, !29}
!22 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !20, file: !1, line: 12, baseType: !10, size: 32)
!23 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !20, file: !1, line: 13, baseType: !24, size: 64, offset: 32)
!24 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bar", file: !1, line: 6, size: 64, elements: !25)
!25 = !{!26, !27}
!26 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !24, file: !1, line: 7, baseType: !10, size: 32)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !24, file: !1, line: 8, baseType: !28, size: 32, offset: 32)
!28 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!29 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !20, file: !1, line: 14, baseType: !30, size: 28800, offset: 96)
!30 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 28800, elements: !31)
!31 = !{!32, !32, !33}
!32 = !DISubrange(count: 10)
!33 = !DISubrange(count: 9)
!34 = !DILocation(line: 20, column: 14, scope: !7)
!35 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 19, type: !10)
!36 = !DILocation(line: 19, column: 7, scope: !7)
!37 = !DILocation(line: 21, column: 8, scope: !38)
!38 = distinct !DILexicalBlock(scope: !7, file: !1, line: 21, column: 3)
!39 = !DILocation(line: 21, column: 17, scope: !40)
!40 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 1)
!41 = distinct !DILexicalBlock(scope: !38, file: !1, line: 21, column: 3)
!42 = !DILocation(line: 21, column: 3, scope: !43)
!43 = !DILexicalBlockFile(scope: !38, file: !1, discriminator: 1)
!44 = !DILocalVariable(name: "j", scope: !7, file: !1, line: 19, type: !10)
!45 = !DILocation(line: 19, column: 10, scope: !7)
!46 = !DILocation(line: 22, column: 10, scope: !47)
!47 = distinct !DILexicalBlock(scope: !48, file: !1, line: 22, column: 5)
!48 = distinct !DILexicalBlock(scope: !41, file: !1, line: 21, column: 35)
!49 = !DILocation(line: 22, column: 19, scope: !50)
!50 = !DILexicalBlockFile(scope: !51, file: !1, discriminator: 1)
!51 = distinct !DILexicalBlock(scope: !47, file: !1, line: 22, column: 5)
!52 = !DILocation(line: 22, column: 5, scope: !53)
!53 = !DILexicalBlockFile(scope: !47, file: !1, discriminator: 1)
!54 = !DILocalVariable(name: "k", scope: !7, file: !1, line: 19, type: !10)
!55 = !DILocation(line: 19, column: 13, scope: !7)
!56 = !DILocation(line: 23, column: 12, scope: !57)
!57 = distinct !DILexicalBlock(scope: !58, file: !1, line: 23, column: 7)
!58 = distinct !DILexicalBlock(scope: !51, file: !1, line: 22, column: 37)
!59 = !DILocation(line: 23, column: 21, scope: !60)
!60 = !DILexicalBlockFile(scope: !61, file: !1, discriminator: 1)
!61 = distinct !DILexicalBlock(scope: !57, file: !1, line: 23, column: 7)
!62 = !DILocation(line: 23, column: 7, scope: !63)
!63 = !DILexicalBlockFile(scope: !57, file: !1, discriminator: 1)
!64 = !DILocation(line: 24, column: 11, scope: !65)
!65 = distinct !DILexicalBlock(scope: !61, file: !1, line: 23, column: 43)
!66 = !DILocation(line: 24, column: 9, scope: !65)
!67 = !DILocation(line: 24, column: 22, scope: !65)
!68 = !DILocation(line: 25, column: 19, scope: !65)
!69 = !DILocation(line: 25, column: 17, scope: !65)
!70 = !DILocation(line: 25, column: 11, scope: !65)
!71 = !DILocation(line: 25, column: 13, scope: !65)
!72 = !DILocation(line: 25, column: 15, scope: !65)
!73 = !DILocation(line: 26, column: 7, scope: !65)
!74 = !DILocation(line: 23, column: 39, scope: !75)
!75 = !DILexicalBlockFile(scope: !61, file: !1, discriminator: 2)
!76 = !DILocation(line: 23, column: 7, scope: !75)
!77 = distinct !{!77, !78, !79}
!78 = !DILocation(line: 23, column: 7, scope: !57)
!79 = !DILocation(line: 26, column: 7, scope: !57)
!80 = !DILocation(line: 27, column: 5, scope: !58)
!81 = !DILocation(line: 22, column: 33, scope: !82)
!82 = !DILexicalBlockFile(scope: !51, file: !1, discriminator: 2)
!83 = !DILocation(line: 22, column: 5, scope: !82)
!84 = distinct !{!84, !85, !86}
!85 = !DILocation(line: 22, column: 5, scope: !47)
!86 = !DILocation(line: 27, column: 5, scope: !47)
!87 = !DILocation(line: 28, column: 3, scope: !48)
!88 = !DILocation(line: 21, column: 31, scope: !89)
!89 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 2)
!90 = !DILocation(line: 21, column: 3, scope: !89)
!91 = distinct !{!91, !92, !93}
!92 = !DILocation(line: 21, column: 3, scope: !38)
!93 = !DILocation(line: 28, column: 3, scope: !38)
!94 = !DILocation(line: 30, column: 8, scope: !95)
!95 = distinct !DILexicalBlock(scope: !7, file: !1, line: 30, column: 3)
!96 = !DILocation(line: 30, column: 17, scope: !97)
!97 = !DILexicalBlockFile(scope: !98, file: !1, discriminator: 1)
!98 = distinct !DILexicalBlock(scope: !95, file: !1, line: 30, column: 3)
!99 = !DILocation(line: 30, column: 3, scope: !100)
!100 = !DILexicalBlockFile(scope: !95, file: !1, discriminator: 1)
!101 = !DILocation(line: 31, column: 22, scope: !102)
!102 = distinct !DILexicalBlock(scope: !98, file: !1, line: 30, column: 35)
!103 = !DILocation(line: 31, column: 20, scope: !102)
!104 = !DILocation(line: 31, column: 32, scope: !102)
!105 = !DILocation(line: 31, column: 5, scope: !102)
!106 = !DILocation(line: 32, column: 3, scope: !102)
!107 = !DILocation(line: 30, column: 31, scope: !108)
!108 = !DILexicalBlockFile(scope: !98, file: !1, discriminator: 2)
!109 = !DILocation(line: 30, column: 3, scope: !108)
!110 = distinct !{!110, !111, !112}
!111 = !DILocation(line: 30, column: 3, scope: !95)
!112 = !DILocation(line: 32, column: 3, scope: !95)
!113 = !DILocation(line: 34, column: 3, scope: !7)
