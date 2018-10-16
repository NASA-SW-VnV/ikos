; ModuleID = 'phi-4.cpp.pp.bc'
source_filename = "phi-4.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

; Function Attrs: noinline norecurse nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !7 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  %7 = alloca [10 x i32], align 16
  %8 = alloca i8, align 1
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !14, metadata !15), !dbg !16
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !17, metadata !15), !dbg !18
  call void @llvm.dbg.declare(metadata i32* %6, metadata !19, metadata !15), !dbg !20
  store i32 0, i32* %6, align 4, !dbg !20
  call void @llvm.dbg.declare(metadata [10 x i32]* %7, metadata !21, metadata !15), !dbg !25
  call void @llvm.dbg.declare(metadata i8* %8, metadata !26, metadata !15), !dbg !28
  %9 = load i32, i32* %4, align 4, !dbg !29
  %10 = srem i32 %9, 5, !dbg !30
  %11 = icmp eq i32 %10, 0, !dbg !31
  %12 = zext i1 %11 to i8, !dbg !28
  store i8 %12, i8* %8, align 1, !dbg !28
  br label %13, !dbg !32

; <label>:13:                                     ; preds = %32, %2
  %14 = load i32, i32* %6, align 4, !dbg !33
  %15 = icmp slt i32 %14, 10, !dbg !37
  br i1 %15, label %16, label %35, !dbg !38

; <label>:16:                                     ; preds = %13
  %17 = load i8, i8* %8, align 1, !dbg !40
  %18 = trunc i8 %17 to i1, !dbg !40
  br i1 %18, label %19, label %25, !dbg !43

; <label>:19:                                     ; preds = %16
  %20 = load i32, i32* %6, align 4, !dbg !44
  %21 = xor i32 %20, 2, !dbg !45
  %22 = load i32, i32* %6, align 4, !dbg !46
  %23 = sext i32 %22 to i64, !dbg !47
  %24 = getelementptr inbounds [10 x i32], [10 x i32]* %7, i64 0, i64 %23, !dbg !47
  store i32 %21, i32* %24, align 4, !dbg !48
  br label %31, !dbg !47

; <label>:25:                                     ; preds = %16
  %26 = load i32, i32* %6, align 4, !dbg !49
  %27 = mul nsw i32 %26, 2, !dbg !50
  %28 = load i32, i32* %6, align 4, !dbg !51
  %29 = sext i32 %28 to i64, !dbg !52
  %30 = getelementptr inbounds [10 x i32], [10 x i32]* %7, i64 0, i64 %29, !dbg !52
  store i32 %27, i32* %30, align 4, !dbg !53
  br label %31

; <label>:31:                                     ; preds = %25, %19
  br label %32, !dbg !54

; <label>:32:                                     ; preds = %31
  %33 = load i32, i32* %6, align 4, !dbg !55
  %34 = add nsw i32 %33, 1, !dbg !55
  store i32 %34, i32* %6, align 4, !dbg !55
  br label %13, !dbg !57, !llvm.loop !58

; <label>:35:                                     ; preds = %13
  ret i32 0, !dbg !61
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si8*** $5 = allocate si8**, 1, align 8
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   [10 x si32]* $7 = allocate [10 x si32], 1, align 16
; CHECK:   ui8* $8 = allocate ui8, 1, align 1
; CHECK:   store $3, 0, align 4
; CHECK:   store $4, %1, align 4
; CHECK:   store $5, %2, align 8
; CHECK:   store $6, 0, align 4
; CHECK:   si32 %9 = load $4, align 4
; CHECK:   si32 %10 = %9 srem 5
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   %10 sieq 0
; CHECK:   ui1 %11 = 1
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   %10 sine 0
; CHECK:   ui1 %11 = 0
; CHECK: }
; CHECK: #4 predecessors={#2, #3} successors={#5} {
; CHECK:   ui8 %12 = zext %11
; CHECK:   store $8, %12, align 1
; CHECK: }
; CHECK: #5 predecessors={#4, #10} successors={#6, #7} {
; CHECK:   si32 %13 = load $6, align 4
; CHECK: }
; CHECK: #6 predecessors={#5} successors={#8, #9} {
; CHECK:   %13 silt 10
; CHECK:   si8* %14 = bitcast $8
; CHECK:   si8 %15 = load %14, align 1
; CHECK:   ui8 %16 = bitcast %15
; CHECK:   ui1 %17 = utrunc %16
; CHECK: }
; CHECK: #7 !exit predecessors={#5} {
; CHECK:   %13 sige 10
; CHECK:   return 0
; CHECK: }
; CHECK: #8 predecessors={#6} successors={#10} {
; CHECK:   ui32* %18 = bitcast $6
; CHECK:   ui32 %19 = load %18, align 4
; CHECK:   ui32 %20 = %19 uxor 2
; CHECK:   si32 %21 = bitcast %20
; CHECK:   si32 %22 = load $6, align 4
; CHECK:   si64 %23 = sext %22
; CHECK:   si32* %24 = ptrshift $7, 40 * 0, 4 * %23
; CHECK:   store %24, %21, align 4
; CHECK: }
; CHECK: #9 predecessors={#6} successors={#10} {
; CHECK:   si32 %25 = load $6, align 4
; CHECK:   si32 %26 = %25 smul.nw 2
; CHECK:   si32 %27 = load $6, align 4
; CHECK:   si64 %28 = sext %27
; CHECK:   si32* %29 = ptrshift $7, 40 * 0, 4 * %28
; CHECK:   store %29, %26, align 4
; CHECK: }
; CHECK: #10 predecessors={#8, #9} successors={#5} {
; CHECK:   si32 %30 = load $6, align 4
; CHECK:   si32 %31 = %30 sadd.nw 1
; CHECK:   store $6, %31, align 4
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline norecurse nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "phi-4.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!14 = !DILocalVariable(name: "argc", arg: 1, scope: !7, file: !1, line: 1, type: !10)
!15 = !DIExpression()
!16 = !DILocation(line: 1, column: 14, scope: !7)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !7, file: !1, line: 1, type: !11)
!18 = !DILocation(line: 1, column: 27, scope: !7)
!19 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 2, type: !10)
!20 = !DILocation(line: 2, column: 7, scope: !7)
!21 = !DILocalVariable(name: "a", scope: !7, file: !1, line: 2, type: !22)
!22 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 320, elements: !23)
!23 = !{!24}
!24 = !DISubrange(count: 10)
!25 = !DILocation(line: 2, column: 14, scope: !7)
!26 = !DILocalVariable(name: "flag", scope: !7, file: !1, line: 3, type: !27)
!27 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!28 = !DILocation(line: 3, column: 8, scope: !7)
!29 = !DILocation(line: 3, column: 15, scope: !7)
!30 = !DILocation(line: 3, column: 20, scope: !7)
!31 = !DILocation(line: 3, column: 24, scope: !7)
!32 = !DILocation(line: 4, column: 3, scope: !7)
!33 = !DILocation(line: 4, column: 10, scope: !34)
!34 = !DILexicalBlockFile(scope: !35, file: !1, discriminator: 1)
!35 = distinct !DILexicalBlock(scope: !36, file: !1, line: 4, column: 3)
!36 = distinct !DILexicalBlock(scope: !7, file: !1, line: 4, column: 3)
!37 = !DILocation(line: 4, column: 12, scope: !34)
!38 = !DILocation(line: 4, column: 3, scope: !39)
!39 = !DILexicalBlockFile(scope: !36, file: !1, discriminator: 1)
!40 = !DILocation(line: 5, column: 9, scope: !41)
!41 = distinct !DILexicalBlock(scope: !42, file: !1, line: 5, column: 9)
!42 = distinct !DILexicalBlock(scope: !35, file: !1, line: 4, column: 23)
!43 = !DILocation(line: 5, column: 9, scope: !42)
!44 = !DILocation(line: 6, column: 14, scope: !41)
!45 = !DILocation(line: 6, column: 16, scope: !41)
!46 = !DILocation(line: 6, column: 9, scope: !41)
!47 = !DILocation(line: 6, column: 7, scope: !41)
!48 = !DILocation(line: 6, column: 12, scope: !41)
!49 = !DILocation(line: 8, column: 14, scope: !41)
!50 = !DILocation(line: 8, column: 16, scope: !41)
!51 = !DILocation(line: 8, column: 9, scope: !41)
!52 = !DILocation(line: 8, column: 7, scope: !41)
!53 = !DILocation(line: 8, column: 12, scope: !41)
!54 = !DILocation(line: 9, column: 3, scope: !42)
!55 = !DILocation(line: 4, column: 19, scope: !56)
!56 = !DILexicalBlockFile(scope: !35, file: !1, discriminator: 2)
!57 = !DILocation(line: 4, column: 3, scope: !56)
!58 = distinct !{!58, !59, !60}
!59 = !DILocation(line: 4, column: 3, scope: !36)
!60 = !DILocation(line: 9, column: 3, scope: !36)
!61 = !DILocation(line: 10, column: 3, scope: !7)
