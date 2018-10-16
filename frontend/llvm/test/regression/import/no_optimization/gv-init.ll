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
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  %7 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !29, metadata !30), !dbg !31
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !32, metadata !30), !dbg !33
  call void @llvm.dbg.declare(metadata i32* %6, metadata !34, metadata !30), !dbg !35
  store i32 0, i32* %6, align 4, !dbg !35
  call void @llvm.dbg.declare(metadata i32* %7, metadata !36, metadata !30), !dbg !37
  store i32 0, i32* %7, align 4, !dbg !37
  br label %8, !dbg !38

; <label>:8:                                      ; preds = %50, %2
  %9 = load i32, i32* %6, align 4, !dbg !39
  %10 = icmp slt i32 %9, 100, !dbg !43
  br i1 %10, label %11, label %53, !dbg !44

; <label>:11:                                     ; preds = %8
  br label %12, !dbg !46

; <label>:12:                                     ; preds = %46, %11
  %13 = load i32, i32* %7, align 4, !dbg !47
  %14 = icmp slt i32 %13, 100, !dbg !51
  br i1 %14, label %15, label %49, !dbg !52

; <label>:15:                                     ; preds = %12
  %16 = load i32, i32* %6, align 4, !dbg !54
  %17 = srem i32 %16, 2, !dbg !56
  %18 = icmp eq i32 %17, 0, !dbg !57
  br i1 %18, label %19, label %32, !dbg !58

; <label>:19:                                     ; preds = %15
  %20 = getelementptr inbounds [2 x i32], [2 x i32]* @b, i64 0, i64 0, !dbg !59
  %21 = load i32, i32* %20, align 4, !dbg !59
  %22 = load i32, i32* @c, align 4, !dbg !60
  %23 = add nsw i32 %21, %22, !dbg !61
  %24 = load i32, i32* @e, align 4, !dbg !62
  %25 = sub nsw i32 %23, %24, !dbg !63
  %26 = load i32, i32* %6, align 4, !dbg !64
  %27 = sext i32 %26 to i64, !dbg !65
  %28 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @a, i64 0, i64 %27, !dbg !65
  %29 = load i32, i32* %7, align 4, !dbg !66
  %30 = sext i32 %29 to i64, !dbg !65
  %31 = getelementptr inbounds [100 x i32], [100 x i32]* %28, i64 0, i64 %30, !dbg !65
  store i32 %25, i32* %31, align 4, !dbg !67
  br label %45, !dbg !65

; <label>:32:                                     ; preds = %15
  %33 = getelementptr inbounds [2 x i32], [2 x i32]* @b, i64 0, i64 1, !dbg !68
  %34 = load i32, i32* %33, align 4, !dbg !68
  %35 = load i32, i32* @d, align 4, !dbg !69
  %36 = add nsw i32 %34, %35, !dbg !70
  %37 = load i32, i32* @e, align 4, !dbg !71
  %38 = sub nsw i32 %36, %37, !dbg !72
  %39 = load i32, i32* %6, align 4, !dbg !73
  %40 = sext i32 %39 to i64, !dbg !74
  %41 = getelementptr inbounds [100 x [100 x i32]], [100 x [100 x i32]]* @a, i64 0, i64 %40, !dbg !74
  %42 = load i32, i32* %7, align 4, !dbg !75
  %43 = sext i32 %42 to i64, !dbg !74
  %44 = getelementptr inbounds [100 x i32], [100 x i32]* %41, i64 0, i64 %43, !dbg !74
  store i32 %38, i32* %44, align 4, !dbg !76
  br label %45

; <label>:45:                                     ; preds = %32, %19
  br label %46, !dbg !77

; <label>:46:                                     ; preds = %45
  %47 = load i32, i32* %7, align 4, !dbg !79
  %48 = add nsw i32 %47, 1, !dbg !79
  store i32 %48, i32* %7, align 4, !dbg !79
  br label %12, !dbg !81, !llvm.loop !82

; <label>:49:                                     ; preds = %12
  br label %50, !dbg !85

; <label>:50:                                     ; preds = %49
  %51 = load i32, i32* %6, align 4, !dbg !86
  %52 = add nsw i32 %51, 1, !dbg !86
  store i32 %52, i32* %6, align 4, !dbg !86
  br label %8, !dbg !88, !llvm.loop !89

; <label>:53:                                     ; preds = %8
  ret i32 0, !dbg !92
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si8*** $5 = allocate si8**, 1, align 8
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   si32* $7 = allocate si32, 1, align 4
; CHECK:   store $3, 0, align 4
; CHECK:   store $4, %1, align 4
; CHECK:   store $5, %2, align 8
; CHECK:   store $6, 0, align 4
; CHECK:   store $7, 0, align 4
; CHECK: }
; CHECK: #2 predecessors={#1, #7} successors={#3, #4} {
; CHECK:   si32 %8 = load $6, align 4
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#5} {
; CHECK:   %8 silt 100
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %8 sige 100
; CHECK:   return 0
; CHECK: }
; CHECK: #5 predecessors={#3, #10} successors={#6, #7} {
; CHECK:   si32 %9 = load $7, align 4
; CHECK: }
; CHECK: #6 predecessors={#5} successors={#8, #9} {
; CHECK:   %9 silt 100
; CHECK:   si32 %10 = load $6, align 4
; CHECK:   si32 %11 = %10 srem 2
; CHECK: }
; CHECK: #7 predecessors={#5} successors={#2} {
; CHECK:   %9 sige 100
; CHECK:   si32 %12 = load $6, align 4
; CHECK:   si32 %13 = %12 sadd.nw 1
; CHECK:   store $6, %13, align 4
; CHECK: }
; CHECK: #8 predecessors={#6} successors={#10} {
; CHECK:   %11 sieq 0
; CHECK:   si32* %14 = ptrshift @b, 8 * 0, 4 * 0
; CHECK:   si32 %15 = load %14, align 4
; CHECK:   si32 %16 = load @c, align 4
; CHECK:   si32 %17 = %15 sadd.nw %16
; CHECK:   si32 %18 = load @e, align 4
; CHECK:   si32 %19 = %17 ssub.nw %18
; CHECK:   si32 %20 = load $6, align 4
; CHECK:   si64 %21 = sext %20
; CHECK:   [100 x si32]* %22 = ptrshift @a, 40000 * 0, 400 * %21
; CHECK:   si32 %23 = load $7, align 4
; CHECK:   si64 %24 = sext %23
; CHECK:   si32* %25 = ptrshift %22, 400 * 0, 4 * %24
; CHECK:   store %25, %19, align 4
; CHECK: }
; CHECK: #9 predecessors={#6} successors={#10} {
; CHECK:   %11 sine 0
; CHECK:   si32* %26 = ptrshift @b, 8 * 0, 4 * 1
; CHECK:   si32 %27 = load %26, align 4
; CHECK:   si32 %28 = load @d, align 4
; CHECK:   si32 %29 = %27 sadd.nw %28
; CHECK:   si32 %30 = load @e, align 4
; CHECK:   si32 %31 = %29 ssub.nw %30
; CHECK:   si32 %32 = load $6, align 4
; CHECK:   si64 %33 = sext %32
; CHECK:   [100 x si32]* %34 = ptrshift @a, 40000 * 0, 400 * %33
; CHECK:   si32 %35 = load $7, align 4
; CHECK:   si64 %36 = sext %35
; CHECK:   si32* %37 = ptrshift %34, 400 * 0, 4 * %36
; CHECK:   store %37, %31, align 4
; CHECK: }
; CHECK: #10 predecessors={#8, #9} successors={#5} {
; CHECK:   si32 %38 = load $7, align 4
; CHECK:   si32 %39 = %38 sadd.nw 1
; CHECK:   store $7, %39, align 4
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!19, !20, !21}
!llvm.ident = !{!22}

!0 = !DIGlobalVariableExpression(var: !1)
!1 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 2, type: !16, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "gv-init.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!39 = !DILocation(line: 9, column: 10, scope: !40)
!40 = !DILexicalBlockFile(scope: !41, file: !3, discriminator: 1)
!41 = distinct !DILexicalBlock(scope: !42, file: !3, line: 9, column: 3)
!42 = distinct !DILexicalBlock(scope: !23, file: !3, line: 9, column: 3)
!43 = !DILocation(line: 9, column: 12, scope: !40)
!44 = !DILocation(line: 9, column: 3, scope: !45)
!45 = !DILexicalBlockFile(scope: !42, file: !3, discriminator: 1)
!46 = !DILocation(line: 10, column: 5, scope: !41)
!47 = !DILocation(line: 10, column: 12, scope: !48)
!48 = !DILexicalBlockFile(scope: !49, file: !3, discriminator: 1)
!49 = distinct !DILexicalBlock(scope: !50, file: !3, line: 10, column: 5)
!50 = distinct !DILexicalBlock(scope: !41, file: !3, line: 10, column: 5)
!51 = !DILocation(line: 10, column: 14, scope: !48)
!52 = !DILocation(line: 10, column: 5, scope: !53)
!53 = !DILexicalBlockFile(scope: !50, file: !3, discriminator: 1)
!54 = !DILocation(line: 11, column: 11, scope: !55)
!55 = distinct !DILexicalBlock(scope: !49, file: !3, line: 11, column: 11)
!56 = !DILocation(line: 11, column: 13, scope: !55)
!57 = !DILocation(line: 11, column: 17, scope: !55)
!58 = !DILocation(line: 11, column: 11, scope: !49)
!59 = !DILocation(line: 12, column: 19, scope: !55)
!60 = !DILocation(line: 12, column: 26, scope: !55)
!61 = !DILocation(line: 12, column: 24, scope: !55)
!62 = !DILocation(line: 12, column: 30, scope: !55)
!63 = !DILocation(line: 12, column: 28, scope: !55)
!64 = !DILocation(line: 12, column: 11, scope: !55)
!65 = !DILocation(line: 12, column: 9, scope: !55)
!66 = !DILocation(line: 12, column: 14, scope: !55)
!67 = !DILocation(line: 12, column: 17, scope: !55)
!68 = !DILocation(line: 14, column: 19, scope: !55)
!69 = !DILocation(line: 14, column: 26, scope: !55)
!70 = !DILocation(line: 14, column: 24, scope: !55)
!71 = !DILocation(line: 14, column: 30, scope: !55)
!72 = !DILocation(line: 14, column: 28, scope: !55)
!73 = !DILocation(line: 14, column: 11, scope: !55)
!74 = !DILocation(line: 14, column: 9, scope: !55)
!75 = !DILocation(line: 14, column: 14, scope: !55)
!76 = !DILocation(line: 14, column: 17, scope: !55)
!77 = !DILocation(line: 11, column: 20, scope: !78)
!78 = !DILexicalBlockFile(scope: !55, file: !3, discriminator: 1)
!79 = !DILocation(line: 10, column: 22, scope: !80)
!80 = !DILexicalBlockFile(scope: !49, file: !3, discriminator: 2)
!81 = !DILocation(line: 10, column: 5, scope: !80)
!82 = distinct !{!82, !83, !84}
!83 = !DILocation(line: 10, column: 5, scope: !50)
!84 = !DILocation(line: 14, column: 30, scope: !50)
!85 = !DILocation(line: 14, column: 30, scope: !53)
!86 = !DILocation(line: 9, column: 20, scope: !87)
!87 = !DILexicalBlockFile(scope: !41, file: !3, discriminator: 2)
!88 = !DILocation(line: 9, column: 3, scope: !87)
!89 = distinct !{!89, !90, !91}
!90 = !DILocation(line: 9, column: 3, scope: !42)
!91 = !DILocation(line: 14, column: 30, scope: !42)
!92 = !DILocation(line: 15, column: 3, scope: !23)
