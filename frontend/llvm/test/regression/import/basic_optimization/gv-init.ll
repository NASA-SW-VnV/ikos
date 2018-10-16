; ModuleID = 'gv-init.c.pp.bc'
source_filename = "gv-init.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

@a = common global [100 x [100 x i32]] zeroinitializer, align 16, !dbg !9
; CHECK: define [100 x [100 x si32]]* @a, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @a, aggregate_zero, align 1
; CHECK: }
; CHECK: }

@b = global [2 x i32] [i32 1, i32 2], align 4, !dbg !0
; CHECK: define [2 x si32]* @b, align 4, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @b, [1, 2], align 1
; CHECK: }
; CHECK: }

@c = common global i32 0, align 4, !dbg !14
; CHECK: define si32* @c, align 4, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @c, 0, align 1
; CHECK: }
; CHECK: }

@d = global i32 5, align 4, !dbg !6
; CHECK: define si32* @d, align 4, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @d, 5, align 1
; CHECK: }
; CHECK: }

@e = external global i32, align 4
; CHECK: declare si32* @e, align 4

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !23 {
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !29, metadata !30), !dbg !31
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !32, metadata !30), !dbg !33
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !34, metadata !30), !dbg !35
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !36, metadata !30), !dbg !37
  br label %3, !dbg !38

; <label>:3:                                      ; preds = %37, %2
  %.01 = phi i32 [ 0, %2 ], [ %38, %37 ]
  %.0 = phi i32 [ 0, %2 ], [ %.1, %37 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !36, metadata !30), !dbg !37
  call void @llvm.dbg.value(metadata i32 %.01, i64 0, metadata !34, metadata !30), !dbg !35
  %4 = icmp slt i32 %.01, 100, !dbg !39
  br i1 %4, label %5, label %39, !dbg !43

; <label>:5:                                      ; preds = %3
  br label %6, !dbg !45

; <label>:6:                                      ; preds = %34, %5
  %.1 = phi i32 [ %.0, %5 ], [ %35, %34 ]
  call void @llvm.dbg.value(metadata i32 %.1, i64 0, metadata !36, metadata !30), !dbg !37
  %7 = icmp slt i32 %.1, 100, !dbg !46
  br i1 %7, label %8, label %36, !dbg !50

; <label>:8:                                      ; preds = %6
  %9 = srem i32 %.01, 2, !dbg !52
  %10 = icmp eq i32 %9, 0, !dbg !54
  br i1 %10, label %11, label %22, !dbg !55

; <label>:11:                                     ; preds = %8
  %12 = getelementptr inbounds [2 x i32], [2 x i32]* @b, i64 0, i64 0, !dbg !56
  %13 = load i32, i32* %12, align 4, !dbg !56
  %14 = load i32, i32* @c, align 4, !dbg !57
  %15 = add nsw i32 %13, %14, !dbg !58
  %16 = load i32, i32* @e, align 4, !dbg !59
  %17 = sub nsw i32 %15, %16, !dbg !60
  %18 = sext i32 %.01 to i64, !dbg !61
  %19 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @a, i64 0, i64 %18, !dbg !61
  %20 = sext i32 %.1 to i64, !dbg !61
  %21 = getelementptr inbounds [100 x i32], [100 x i32]* %19, i64 0, i64 %20, !dbg !61
  store i32 %17, i32* %21, align 4, !dbg !62
  br label %33, !dbg !61

; <label>:22:                                     ; preds = %8
  %23 = getelementptr inbounds [2 x i32], [2 x i32]* @b, i64 0, i64 1, !dbg !63
  %24 = load i32, i32* %23, align 4, !dbg !63
  %25 = load i32, i32* @d, align 4, !dbg !64
  %26 = add nsw i32 %24, %25, !dbg !65
  %27 = load i32, i32* @e, align 4, !dbg !66
  %28 = sub nsw i32 %26, %27, !dbg !67
  %29 = sext i32 %.01 to i64, !dbg !68
  %30 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @a, i64 0, i64 %29, !dbg !68
  %31 = sext i32 %.1 to i64, !dbg !68
  %32 = getelementptr inbounds [100 x i32], [100 x i32]* %30, i64 0, i64 %31, !dbg !68
  store i32 %28, i32* %32, align 4, !dbg !69
  br label %33

; <label>:33:                                     ; preds = %22, %11
  br label %34, !dbg !70

; <label>:34:                                     ; preds = %33
  %35 = add nsw i32 %.1, 1, !dbg !72
  call void @llvm.dbg.value(metadata i32 %35, i64 0, metadata !36, metadata !30), !dbg !37
  br label %6, !dbg !74, !llvm.loop !75

; <label>:36:                                     ; preds = %6
  br label %37, !dbg !78

; <label>:37:                                     ; preds = %36
  %38 = add nsw i32 %.01, 1, !dbg !79
  call void @llvm.dbg.value(metadata i32 %38, i64 0, metadata !34, metadata !30), !dbg !35
  br label %3, !dbg !81, !llvm.loop !82

; <label>:39:                                     ; preds = %3
  ret i32 0, !dbg !85
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32 %.01 = 0
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #7} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#5} {
; CHECK:   %.01 silt 100
; CHECK:   si32 %.1 = %.0
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %.01 sige 100
; CHECK:   return 0
; CHECK: }
; CHECK: #5 predecessors={#3, #10} successors={#6, #7} {
; CHECK: }
; CHECK: #6 predecessors={#5} successors={#8, #9} {
; CHECK:   %.1 silt 100
; CHECK:   si32 %3 = %.01 srem 2
; CHECK: }
; CHECK: #7 predecessors={#5} successors={#2} {
; CHECK:   %.1 sige 100
; CHECK:   si32 %4 = %.01 sadd.nw 1
; CHECK:   si32 %.01 = %4
; CHECK:   si32 %.0 = %.1
; CHECK: }
; CHECK: #8 predecessors={#6} successors={#10} {
; CHECK:   %3 sieq 0
; CHECK:   si32* %5 = ptrshift @b, 8 * 0, 4 * 0
; CHECK:   si32 %6 = load %5, align 4
; CHECK:   si32 %7 = load @c, align 4
; CHECK:   si32 %8 = %6 sadd.nw %7
; CHECK:   si32 %9 = load @e, align 4
; CHECK:   si32 %10 = %8 ssub.nw %9
; CHECK:   si64 %11 = sext %.01
; CHECK:   [100 x si32]* %12 = ptrshift @a, 40000 * 0, 400 * %11
; CHECK:   si64 %13 = sext %.1
; CHECK:   si32* %14 = ptrshift %12, 400 * 0, 4 * %13
; CHECK:   store %14, %10, align 4
; CHECK: }
; CHECK: #9 predecessors={#6} successors={#10} {
; CHECK:   %3 sine 0
; CHECK:   si32* %15 = ptrshift @b, 8 * 0, 4 * 1
; CHECK:   si32 %16 = load %15, align 4
; CHECK:   si32 %17 = load @d, align 4
; CHECK:   si32 %18 = %16 sadd.nw %17
; CHECK:   si32 %19 = load @e, align 4
; CHECK:   si32 %20 = %18 ssub.nw %19
; CHECK:   si64 %21 = sext %.01
; CHECK:   [100 x si32]* %22 = ptrshift @a, 40000 * 0, 400 * %21
; CHECK:   si64 %23 = sext %.1
; CHECK:   si32* %24 = ptrshift %22, 400 * 0, 4 * %23
; CHECK:   store %24, %20, align 4
; CHECK: }
; CHECK: #10 predecessors={#8, #9} successors={#5} {
; CHECK:   si32 %25 = %.1 sadd.nw 1
; CHECK:   si32 %.1 = %25
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!19, !20, !21}
!llvm.ident = !{!22}

!0 = !DIGlobalVariableExpression(var: !1)
!1 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 2, type: !16, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "gv-init.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
!4 = !{}
!5 = !{!0, !6, !9, !14}
!6 = !DIGlobalVariableExpression(var: !7)
!7 = distinct !DIGlobalVariable(name: "d", scope: !2, file: !3, line: 4, type: !8, isLocal: false, isDefinition: true)
!8 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!9 = !DIGlobalVariableExpression(var: !10)
!10 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !11, isLocal: false, isDefinition: true)
!11 = !DICompositeType(tag: DW_TAG_array_type, baseType: !8, size: 320000, elements: !12)
!12 = !{!13, !13}
!13 = !DISubrange(count: 100)
!14 = !DIGlobalVariableExpression(var: !15)
!15 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!16 = !DICompositeType(tag: DW_TAG_array_type, baseType: !8, size: 64, elements: !17)
!17 = !{!18}
!18 = !DISubrange(count: 2)
!19 = !{i32 2, !"Dwarf Version", i32 4}
!20 = !{i32 2, !"Debug Info Version", i32 3}
!21 = !{i32 1, !"PIC Level", i32 2}
!22 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!23 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 7, type: !24, isLocal: false, isDefinition: true, scopeLine: 7, flags: DIFlagPrototyped, isOptimized: false, unit: !2, variables: !4)
!24 = !DISubroutineType(types: !25)
!25 = !{!8, !8, !26}
!26 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !27, size: 64)
!27 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !28, size: 64)
!28 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!29 = !DILocalVariable(name: "argc", arg: 1, scope: !23, file: !3, line: 7, type: !8)
!30 = !DIExpression()
!31 = !DILocation(line: 7, column: 14, scope: !23)
!32 = !DILocalVariable(name: "argv", arg: 2, scope: !23, file: !3, line: 7, type: !26)
!33 = !DILocation(line: 7, column: 27, scope: !23)
!34 = !DILocalVariable(name: "i", scope: !23, file: !3, line: 8, type: !8)
!35 = !DILocation(line: 8, column: 7, scope: !23)
!36 = !DILocalVariable(name: "j", scope: !23, file: !3, line: 8, type: !8)
!37 = !DILocation(line: 8, column: 14, scope: !23)
!38 = !DILocation(line: 9, column: 3, scope: !23)
!39 = !DILocation(line: 9, column: 12, scope: !40)
!40 = !DILexicalBlockFile(scope: !41, file: !3, discriminator: 1)
!41 = distinct !DILexicalBlock(scope: !42, file: !3, line: 9, column: 3)
!42 = distinct !DILexicalBlock(scope: !23, file: !3, line: 9, column: 3)
!43 = !DILocation(line: 9, column: 3, scope: !44)
!44 = !DILexicalBlockFile(scope: !42, file: !3, discriminator: 1)
!45 = !DILocation(line: 10, column: 5, scope: !41)
!46 = !DILocation(line: 10, column: 14, scope: !47)
!47 = !DILexicalBlockFile(scope: !48, file: !3, discriminator: 1)
!48 = distinct !DILexicalBlock(scope: !49, file: !3, line: 10, column: 5)
!49 = distinct !DILexicalBlock(scope: !41, file: !3, line: 10, column: 5)
!50 = !DILocation(line: 10, column: 5, scope: !51)
!51 = !DILexicalBlockFile(scope: !49, file: !3, discriminator: 1)
!52 = !DILocation(line: 11, column: 13, scope: !53)
!53 = distinct !DILexicalBlock(scope: !48, file: !3, line: 11, column: 11)
!54 = !DILocation(line: 11, column: 17, scope: !53)
!55 = !DILocation(line: 11, column: 11, scope: !48)
!56 = !DILocation(line: 12, column: 19, scope: !53)
!57 = !DILocation(line: 12, column: 26, scope: !53)
!58 = !DILocation(line: 12, column: 24, scope: !53)
!59 = !DILocation(line: 12, column: 30, scope: !53)
!60 = !DILocation(line: 12, column: 28, scope: !53)
!61 = !DILocation(line: 12, column: 9, scope: !53)
!62 = !DILocation(line: 12, column: 17, scope: !53)
!63 = !DILocation(line: 14, column: 19, scope: !53)
!64 = !DILocation(line: 14, column: 26, scope: !53)
!65 = !DILocation(line: 14, column: 24, scope: !53)
!66 = !DILocation(line: 14, column: 30, scope: !53)
!67 = !DILocation(line: 14, column: 28, scope: !53)
!68 = !DILocation(line: 14, column: 9, scope: !53)
!69 = !DILocation(line: 14, column: 17, scope: !53)
!70 = !DILocation(line: 11, column: 20, scope: !71)
!71 = !DILexicalBlockFile(scope: !53, file: !3, discriminator: 1)
!72 = !DILocation(line: 10, column: 22, scope: !73)
!73 = !DILexicalBlockFile(scope: !48, file: !3, discriminator: 2)
!74 = !DILocation(line: 10, column: 5, scope: !73)
!75 = distinct !{!75, !76, !77}
!76 = !DILocation(line: 10, column: 5, scope: !49)
!77 = !DILocation(line: 14, column: 30, scope: !49)
!78 = !DILocation(line: 14, column: 30, scope: !51)
!79 = !DILocation(line: 9, column: 20, scope: !80)
!80 = !DILexicalBlockFile(scope: !41, file: !3, discriminator: 2)
!81 = !DILocation(line: 9, column: 3, scope: !80)
!82 = distinct !{!82, !83, !84}
!83 = !DILocation(line: 9, column: 3, scope: !42)
!84 = !DILocation(line: 14, column: 30, scope: !42)
!85 = !DILocation(line: 15, column: 3, scope: !23)
