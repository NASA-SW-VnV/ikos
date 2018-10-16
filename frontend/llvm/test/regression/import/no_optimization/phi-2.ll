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
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  %8 = alloca i32, align 4
  %9 = alloca %struct.foo, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !14, metadata !15), !dbg !16
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !17, metadata !15), !dbg !18
  call void @llvm.dbg.declare(metadata i32* %6, metadata !19, metadata !15), !dbg !20
  call void @llvm.dbg.declare(metadata i32* %7, metadata !21, metadata !15), !dbg !22
  call void @llvm.dbg.declare(metadata i32* %8, metadata !23, metadata !15), !dbg !24
  call void @llvm.dbg.declare(metadata %struct.foo* %9, metadata !25, metadata !15), !dbg !40
  store i32 0, i32* %6, align 4, !dbg !41
  br label %10, !dbg !43

; <label>:10:                                     ; preds = %54, %2
  %11 = load i32, i32* %6, align 4, !dbg !44
  %12 = icmp slt i32 %11, 10, !dbg !47
  br i1 %12, label %13, label %57, !dbg !48

; <label>:13:                                     ; preds = %10
  store i32 0, i32* %7, align 4, !dbg !50
  br label %14, !dbg !53

; <label>:14:                                     ; preds = %50, %13
  %15 = load i32, i32* %7, align 4, !dbg !54
  %16 = icmp slt i32 %15, 10, !dbg !57
  br i1 %16, label %17, label %53, !dbg !58

; <label>:17:                                     ; preds = %14
  store i32 0, i32* %8, align 4, !dbg !60
  br label %18, !dbg !63

; <label>:18:                                     ; preds = %46, %17
  %19 = load i32, i32* %8, align 4, !dbg !64
  %20 = icmp slt i32 %19, 9, !dbg !67
  br i1 %20, label %21, label %49, !dbg !68

; <label>:21:                                     ; preds = %18
  %22 = load i32, i32* %4, align 4, !dbg !70
  %23 = getelementptr inbounds %struct.foo, %struct.foo* %9, i32 0, i32 2, !dbg !72
  %24 = load i32, i32* %6, align 4, !dbg !73
  %25 = sext i32 %24 to i64, !dbg !74
  %26 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %23, i64 0, i64 %25, !dbg !74
  %27 = load i32, i32* %7, align 4, !dbg !75
  %28 = sext i32 %27 to i64, !dbg !74
  %29 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %26, i64 0, i64 %28, !dbg !74
  %30 = load i32, i32* %8, align 4, !dbg !76
  %31 = sext i32 %30 to i64, !dbg !74
  %32 = getelementptr inbounds [9 x i32], [9 x i32]* %29, i64 0, i64 %31, !dbg !74
  store i32 %22, i32* %32, align 4, !dbg !77
  %33 = getelementptr inbounds %struct.foo, %struct.foo* %9, i32 0, i32 2, !dbg !78
  %34 = load i32, i32* %6, align 4, !dbg !79
  %35 = sext i32 %34 to i64, !dbg !80
  %36 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %33, i64 0, i64 %35, !dbg !80
  %37 = load i32, i32* %7, align 4, !dbg !81
  %38 = sext i32 %37 to i64, !dbg !80
  %39 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %36, i64 0, i64 %38, !dbg !80
  %40 = load i32, i32* %8, align 4, !dbg !82
  %41 = sext i32 %40 to i64, !dbg !80
  %42 = getelementptr inbounds [9 x i32], [9 x i32]* %39, i64 0, i64 %41, !dbg !80
  %43 = load i32, i32* %42, align 4, !dbg !80
  %44 = getelementptr inbounds %struct.foo, %struct.foo* %9, i32 0, i32 1, !dbg !83
  %45 = getelementptr inbounds %struct.bar, %struct.bar* %44, i32 0, i32 0, !dbg !84
  store i32 %43, i32* %45, align 4, !dbg !85
  br label %46, !dbg !86

; <label>:46:                                     ; preds = %21
  %47 = load i32, i32* %8, align 4, !dbg !87
  %48 = add nsw i32 %47, 1, !dbg !87
  store i32 %48, i32* %8, align 4, !dbg !87
  br label %18, !dbg !89, !llvm.loop !90

; <label>:49:                                     ; preds = %18
  br label %50, !dbg !93

; <label>:50:                                     ; preds = %49
  %51 = load i32, i32* %7, align 4, !dbg !94
  %52 = add nsw i32 %51, 1, !dbg !94
  store i32 %52, i32* %7, align 4, !dbg !94
  br label %14, !dbg !96, !llvm.loop !97

; <label>:53:                                     ; preds = %14
  br label %54, !dbg !100

; <label>:54:                                     ; preds = %53
  %55 = load i32, i32* %6, align 4, !dbg !101
  %56 = add nsw i32 %55, 1, !dbg !101
  store i32 %56, i32* %6, align 4, !dbg !101
  br label %10, !dbg !103, !llvm.loop !104

; <label>:57:                                     ; preds = %10
  store i32 0, i32* %6, align 4, !dbg !107
  br label %58, !dbg !109

; <label>:58:                                     ; preds = %76, %57
  %59 = load i32, i32* %6, align 4, !dbg !110
  %60 = icmp slt i32 %59, 10, !dbg !113
  br i1 %60, label %61, label %79, !dbg !114

; <label>:61:                                     ; preds = %58
  %62 = getelementptr inbounds %struct.foo, %struct.foo* %9, i32 0, i32 2, !dbg !116
  %63 = load i32, i32* %6, align 4, !dbg !118
  %64 = sext i32 %63 to i64, !dbg !119
  %65 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %62, i64 0, i64 %64, !dbg !119
  %66 = load i32, i32* %6, align 4, !dbg !120
  %67 = sext i32 %66 to i64, !dbg !119
  %68 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %65, i64 0, i64 %67, !dbg !119
  %69 = load i32, i32* %6, align 4, !dbg !121
  %70 = sub nsw i32 %69, 1, !dbg !122
  %71 = sext i32 %70 to i64, !dbg !119
  %72 = getelementptr inbounds [9 x i32], [9 x i32]* %68, i64 0, i64 %71, !dbg !119
  %73 = load i32, i32* %72, align 4, !dbg !119
  %74 = getelementptr inbounds [4 x i8], [4 x i8]* @.str, i32 0, i32 0, !dbg !123
  %75 = call i32 (i8*, ...) @printf(i8* %74, i32 %73), !dbg !123
  br label %76, !dbg !124

; <label>:76:                                     ; preds = %61
  %77 = load i32, i32* %6, align 4, !dbg !125
  %78 = add nsw i32 %77, 1, !dbg !125
  store i32 %78, i32* %6, align 4, !dbg !125
  br label %58, !dbg !127, !llvm.loop !128

; <label>:79:                                     ; preds = %58
  ret i32 0, !dbg !131
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si8*** $5 = allocate si8**, 1, align 8
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   si32* $7 = allocate si32, 1, align 4
; CHECK:   si32* $8 = allocate si32, 1, align 4
; CHECK:   {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}* $9 = allocate {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}, 1, align 4
; CHECK:   store $3, 0, align 4
; CHECK:   store $4, %1, align 4
; CHECK:   store $5, %2, align 8
; CHECK:   store $6, 0, align 4
; CHECK: }
; CHECK: #2 predecessors={#1, #8} successors={#3, #4} {
; CHECK:   si32 %10 = load $6, align 4
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#5} {
; CHECK:   %10 silt 10
; CHECK:   store $7, 0, align 4
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#6} {
; CHECK:   %10 sige 10
; CHECK:   store $6, 0, align 4
; CHECK: }
; CHECK: #5 predecessors={#3, #13} successors={#7, #8} {
; CHECK:   si32 %11 = load $7, align 4
; CHECK: }
; CHECK: #7 predecessors={#5} successors={#11} {
; CHECK:   %11 silt 10
; CHECK:   store $8, 0, align 4
; CHECK: }
; CHECK: #8 predecessors={#5} successors={#2} {
; CHECK:   %11 sige 10
; CHECK:   si32 %13 = load $6, align 4
; CHECK:   si32 %14 = %13 sadd.nw 1
; CHECK:   store $6, %14, align 4
; CHECK: }
; CHECK: #6 predecessors={#4, #9} successors={#9, #10} {
; CHECK:   si32 %12 = load $6, align 4
; CHECK: }
; CHECK: #9 predecessors={#6} successors={#6} {
; CHECK:   %12 silt 10
; CHECK:   [10 x [10 x [9 x si32]]]* %15 = ptrshift $9, 3612 * 0, 1 * 12
; CHECK:   si32 %16 = load $6, align 4
; CHECK:   si64 %17 = sext %16
; CHECK:   [10 x [9 x si32]]* %18 = ptrshift %15, 3600 * 0, 360 * %17
; CHECK:   si32 %19 = load $6, align 4
; CHECK:   si64 %20 = sext %19
; CHECK:   [9 x si32]* %21 = ptrshift %18, 360 * 0, 36 * %20
; CHECK:   si32 %22 = load $6, align 4
; CHECK:   si32 %23 = %22 ssub.nw 1
; CHECK:   si64 %24 = sext %23
; CHECK:   si32* %25 = ptrshift %21, 36 * 0, 4 * %24
; CHECK:   si32 %26 = load %25, align 4
; CHECK:   si8* %27 = ptrshift @.str, 4 * 0, 1 * 0
; CHECK:   si32 %28 = call @ar.libc.printf(%27, %26)
; CHECK:   si32 %29 = load $6, align 4
; CHECK:   si32 %30 = %29 sadd.nw 1
; CHECK:   store $6, %30, align 4
; CHECK: }
; CHECK: #10 !exit predecessors={#6} {
; CHECK:   %12 sige 10
; CHECK:   return 0
; CHECK: }
; CHECK: #11 predecessors={#7, #12} successors={#12, #13} {
; CHECK:   si32 %31 = load $8, align 4
; CHECK: }
; CHECK: #12 predecessors={#11} successors={#11} {
; CHECK:   %31 silt 9
; CHECK:   si32 %32 = load $4, align 4
; CHECK:   [10 x [10 x [9 x si32]]]* %33 = ptrshift $9, 3612 * 0, 1 * 12
; CHECK:   si32 %34 = load $6, align 4
; CHECK:   si64 %35 = sext %34
; CHECK:   [10 x [9 x si32]]* %36 = ptrshift %33, 3600 * 0, 360 * %35
; CHECK:   si32 %37 = load $7, align 4
; CHECK:   si64 %38 = sext %37
; CHECK:   [9 x si32]* %39 = ptrshift %36, 360 * 0, 36 * %38
; CHECK:   si32 %40 = load $8, align 4
; CHECK:   si64 %41 = sext %40
; CHECK:   si32* %42 = ptrshift %39, 36 * 0, 4 * %41
; CHECK:   store %42, %32, align 4
; CHECK:   [10 x [10 x [9 x si32]]]* %43 = ptrshift $9, 3612 * 0, 1 * 12
; CHECK:   si32 %44 = load $6, align 4
; CHECK:   si64 %45 = sext %44
; CHECK:   [10 x [9 x si32]]* %46 = ptrshift %43, 3600 * 0, 360 * %45
; CHECK:   si32 %47 = load $7, align 4
; CHECK:   si64 %48 = sext %47
; CHECK:   [9 x si32]* %49 = ptrshift %46, 360 * 0, 36 * %48
; CHECK:   si32 %50 = load $8, align 4
; CHECK:   si64 %51 = sext %50
; CHECK:   si32* %52 = ptrshift %49, 36 * 0, 4 * %51
; CHECK:   si32 %53 = load %52, align 4
; CHECK:   {0: si32, 4: float}* %54 = ptrshift $9, 3612 * 0, 1 * 4
; CHECK:   si32* %55 = ptrshift %54, 8 * 0, 1 * 0
; CHECK:   store %55, %53, align 4
; CHECK:   si32 %56 = load $8, align 4
; CHECK:   si32 %57 = %56 sadd.nw 1
; CHECK:   store $8, %57, align 4
; CHECK: }
; CHECK: #13 predecessors={#11} successors={#5} {
; CHECK:   %31 sige 9
; CHECK:   si32 %58 = load $7, align 4
; CHECK:   si32 %59 = %58 sadd.nw 1
; CHECK:   store $7, %59, align 4
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "phi-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!19 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 19, type: !10)
!20 = !DILocation(line: 19, column: 7, scope: !7)
!21 = !DILocalVariable(name: "j", scope: !7, file: !1, line: 19, type: !10)
!22 = !DILocation(line: 19, column: 10, scope: !7)
!23 = !DILocalVariable(name: "k", scope: !7, file: !1, line: 19, type: !10)
!24 = !DILocation(line: 19, column: 13, scope: !7)
!25 = !DILocalVariable(name: "x", scope: !7, file: !1, line: 20, type: !26)
!26 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "foo", file: !1, line: 11, size: 28896, elements: !27)
!27 = !{!28, !29, !35}
!28 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !26, file: !1, line: 12, baseType: !10, size: 32)
!29 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !26, file: !1, line: 13, baseType: !30, size: 64, offset: 32)
!30 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "bar", file: !1, line: 6, size: 64, elements: !31)
!31 = !{!32, !33}
!32 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !30, file: !1, line: 7, baseType: !10, size: 32)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !30, file: !1, line: 8, baseType: !34, size: 32, offset: 32)
!34 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!35 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !26, file: !1, line: 14, baseType: !36, size: 28800, offset: 96)
!36 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 28800, elements: !37)
!37 = !{!38, !38, !39}
!38 = !DISubrange(count: 10)
!39 = !DISubrange(count: 9)
!40 = !DILocation(line: 20, column: 14, scope: !7)
!41 = !DILocation(line: 21, column: 10, scope: !42)
!42 = distinct !DILexicalBlock(scope: !7, file: !1, line: 21, column: 3)
!43 = !DILocation(line: 21, column: 8, scope: !42)
!44 = !DILocation(line: 21, column: 15, scope: !45)
!45 = !DILexicalBlockFile(scope: !46, file: !1, discriminator: 1)
!46 = distinct !DILexicalBlock(scope: !42, file: !1, line: 21, column: 3)
!47 = !DILocation(line: 21, column: 17, scope: !45)
!48 = !DILocation(line: 21, column: 3, scope: !49)
!49 = !DILexicalBlockFile(scope: !42, file: !1, discriminator: 1)
!50 = !DILocation(line: 22, column: 12, scope: !51)
!51 = distinct !DILexicalBlock(scope: !52, file: !1, line: 22, column: 5)
!52 = distinct !DILexicalBlock(scope: !46, file: !1, line: 21, column: 35)
!53 = !DILocation(line: 22, column: 10, scope: !51)
!54 = !DILocation(line: 22, column: 17, scope: !55)
!55 = !DILexicalBlockFile(scope: !56, file: !1, discriminator: 1)
!56 = distinct !DILexicalBlock(scope: !51, file: !1, line: 22, column: 5)
!57 = !DILocation(line: 22, column: 19, scope: !55)
!58 = !DILocation(line: 22, column: 5, scope: !59)
!59 = !DILexicalBlockFile(scope: !51, file: !1, discriminator: 1)
!60 = !DILocation(line: 23, column: 14, scope: !61)
!61 = distinct !DILexicalBlock(scope: !62, file: !1, line: 23, column: 7)
!62 = distinct !DILexicalBlock(scope: !56, file: !1, line: 22, column: 37)
!63 = !DILocation(line: 23, column: 12, scope: !61)
!64 = !DILocation(line: 23, column: 19, scope: !65)
!65 = !DILexicalBlockFile(scope: !66, file: !1, discriminator: 1)
!66 = distinct !DILexicalBlock(scope: !61, file: !1, line: 23, column: 7)
!67 = !DILocation(line: 23, column: 21, scope: !65)
!68 = !DILocation(line: 23, column: 7, scope: !69)
!69 = !DILexicalBlockFile(scope: !61, file: !1, discriminator: 1)
!70 = !DILocation(line: 24, column: 24, scope: !71)
!71 = distinct !DILexicalBlock(scope: !66, file: !1, line: 23, column: 43)
!72 = !DILocation(line: 24, column: 11, scope: !71)
!73 = !DILocation(line: 24, column: 13, scope: !71)
!74 = !DILocation(line: 24, column: 9, scope: !71)
!75 = !DILocation(line: 24, column: 16, scope: !71)
!76 = !DILocation(line: 24, column: 19, scope: !71)
!77 = !DILocation(line: 24, column: 22, scope: !71)
!78 = !DILocation(line: 25, column: 19, scope: !71)
!79 = !DILocation(line: 25, column: 21, scope: !71)
!80 = !DILocation(line: 25, column: 17, scope: !71)
!81 = !DILocation(line: 25, column: 24, scope: !71)
!82 = !DILocation(line: 25, column: 27, scope: !71)
!83 = !DILocation(line: 25, column: 11, scope: !71)
!84 = !DILocation(line: 25, column: 13, scope: !71)
!85 = !DILocation(line: 25, column: 15, scope: !71)
!86 = !DILocation(line: 26, column: 7, scope: !71)
!87 = !DILocation(line: 23, column: 39, scope: !88)
!88 = !DILexicalBlockFile(scope: !66, file: !1, discriminator: 2)
!89 = !DILocation(line: 23, column: 7, scope: !88)
!90 = distinct !{!90, !91, !92}
!91 = !DILocation(line: 23, column: 7, scope: !61)
!92 = !DILocation(line: 26, column: 7, scope: !61)
!93 = !DILocation(line: 27, column: 5, scope: !62)
!94 = !DILocation(line: 22, column: 33, scope: !95)
!95 = !DILexicalBlockFile(scope: !56, file: !1, discriminator: 2)
!96 = !DILocation(line: 22, column: 5, scope: !95)
!97 = distinct !{!97, !98, !99}
!98 = !DILocation(line: 22, column: 5, scope: !51)
!99 = !DILocation(line: 27, column: 5, scope: !51)
!100 = !DILocation(line: 28, column: 3, scope: !52)
!101 = !DILocation(line: 21, column: 31, scope: !102)
!102 = !DILexicalBlockFile(scope: !46, file: !1, discriminator: 2)
!103 = !DILocation(line: 21, column: 3, scope: !102)
!104 = distinct !{!104, !105, !106}
!105 = !DILocation(line: 21, column: 3, scope: !42)
!106 = !DILocation(line: 28, column: 3, scope: !42)
!107 = !DILocation(line: 30, column: 10, scope: !108)
!108 = distinct !DILexicalBlock(scope: !7, file: !1, line: 30, column: 3)
!109 = !DILocation(line: 30, column: 8, scope: !108)
!110 = !DILocation(line: 30, column: 15, scope: !111)
!111 = !DILexicalBlockFile(scope: !112, file: !1, discriminator: 1)
!112 = distinct !DILexicalBlock(scope: !108, file: !1, line: 30, column: 3)
!113 = !DILocation(line: 30, column: 17, scope: !111)
!114 = !DILocation(line: 30, column: 3, scope: !115)
!115 = !DILexicalBlockFile(scope: !108, file: !1, discriminator: 1)
!116 = !DILocation(line: 31, column: 22, scope: !117)
!117 = distinct !DILexicalBlock(scope: !112, file: !1, line: 30, column: 35)
!118 = !DILocation(line: 31, column: 24, scope: !117)
!119 = !DILocation(line: 31, column: 20, scope: !117)
!120 = !DILocation(line: 31, column: 27, scope: !117)
!121 = !DILocation(line: 31, column: 30, scope: !117)
!122 = !DILocation(line: 31, column: 32, scope: !117)
!123 = !DILocation(line: 31, column: 5, scope: !117)
!124 = !DILocation(line: 32, column: 3, scope: !117)
!125 = !DILocation(line: 30, column: 31, scope: !126)
!126 = !DILexicalBlockFile(scope: !112, file: !1, discriminator: 2)
!127 = !DILocation(line: 30, column: 3, scope: !126)
!128 = distinct !{!128, !129, !130}
!129 = !DILocation(line: 30, column: 3, scope: !108)
!130 = !DILocation(line: 32, column: 3, scope: !108)
!131 = !DILocation(line: 34, column: 3, scope: !7)
