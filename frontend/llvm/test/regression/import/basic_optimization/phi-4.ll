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
  %3 = alloca [10 x i32], align 16
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !14, metadata !15), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !17, metadata !15), !dbg !18
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !19, metadata !15), !dbg !20
  call void @llvm.dbg.declare(metadata [10 x i32]* %3, metadata !21, metadata !15), !dbg !25
  %4 = srem i32 %0, 5, !dbg !26
  %5 = icmp eq i32 %4, 0, !dbg !27
  %6 = zext i1 %5 to i8, !dbg !28
  call void @llvm.dbg.value(metadata i8 %6, i64 0, metadata !29, metadata !15), !dbg !28
  br label %7, !dbg !31

; <label>:7:                                      ; preds = %20, %2
  %.0 = phi i32 [ 0, %2 ], [ %21, %20 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !19, metadata !15), !dbg !20
  %8 = icmp slt i32 %.0, 10, !dbg !32
  br i1 %8, label %9, label %22, !dbg !36

; <label>:9:                                      ; preds = %7
  %10 = trunc i8 %6 to i1, !dbg !38
  br i1 %10, label %11, label %15, !dbg !41

; <label>:11:                                     ; preds = %9
  %12 = xor i32 %.0, 2, !dbg !42
  %13 = sext i32 %.0 to i64, !dbg !43
  %14 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 %13, !dbg !43
  store i32 %12, i32* %14, align 4, !dbg !44
  br label %19, !dbg !43

; <label>:15:                                     ; preds = %9
  %16 = mul nsw i32 %.0, 2, !dbg !45
  %17 = sext i32 %.0 to i64, !dbg !46
  %18 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 %17, !dbg !46
  store i32 %16, i32* %18, align 4, !dbg !47
  br label %19

; <label>:19:                                     ; preds = %15, %11
  br label %20, !dbg !48

; <label>:20:                                     ; preds = %19
  %21 = add nsw i32 %.0, 1, !dbg !49
  call void @llvm.dbg.value(metadata i32 %21, i64 0, metadata !19, metadata !15), !dbg !20
  br label %7, !dbg !51, !llvm.loop !52

; <label>:22:                                     ; preds = %7
  ret i32 0, !dbg !55
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   [10 x si32]* $3 = allocate [10 x si32], 1, align 16
; CHECK:   si32 %4 = %1 srem 5
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4} {
; CHECK:   %4 sieq 0
; CHECK:   ui1 %5 = 1
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#4} {
; CHECK:   %4 sine 0
; CHECK:   ui1 %5 = 0
; CHECK: }
; CHECK: #4 predecessors={#2, #3} successors={#5} {
; CHECK:   ui8 %6 = zext %5
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #5 predecessors={#4, #10} successors={#6, #7} {
; CHECK: }
; CHECK: #6 predecessors={#5} successors={#8, #9} {
; CHECK:   %.0 silt 10
; CHECK:   ui1 %7 = utrunc %6
; CHECK: }
; CHECK: #7 !exit predecessors={#5} {
; CHECK:   %.0 sige 10
; CHECK:   return 0
; CHECK: }
; CHECK: #8 predecessors={#6} successors={#10} {
; CHECK:   si32 %8 = %.0 sxor 2
; CHECK:   si64 %9 = sext %.0
; CHECK:   si32* %10 = ptrshift $3, 40 * 0, 4 * %9
; CHECK:   store %10, %8, align 4
; CHECK: }
; CHECK: #9 predecessors={#6} successors={#10} {
; CHECK:   si32 %11 = %.0 smul.nw 2
; CHECK:   si64 %12 = sext %.0
; CHECK:   si32* %13 = ptrshift $3, 40 * 0, 4 * %12
; CHECK:   store %13, %11, align 4
; CHECK: }
; CHECK: #10 predecessors={#8, #9} successors={#5} {
; CHECK:   si32 %14 = %.0 sadd.nw 1
; CHECK:   si32 %.0 = %14
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline norecurse nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "phi-4.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
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
!26 = !DILocation(line: 3, column: 20, scope: !7)
!27 = !DILocation(line: 3, column: 24, scope: !7)
!28 = !DILocation(line: 3, column: 8, scope: !7)
!29 = !DILocalVariable(name: "flag", scope: !7, file: !1, line: 3, type: !30)
!30 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!31 = !DILocation(line: 4, column: 3, scope: !7)
!32 = !DILocation(line: 4, column: 12, scope: !33)
!33 = !DILexicalBlockFile(scope: !34, file: !1, discriminator: 1)
!34 = distinct !DILexicalBlock(scope: !35, file: !1, line: 4, column: 3)
!35 = distinct !DILexicalBlock(scope: !7, file: !1, line: 4, column: 3)
!36 = !DILocation(line: 4, column: 3, scope: !37)
!37 = !DILexicalBlockFile(scope: !35, file: !1, discriminator: 1)
!38 = !DILocation(line: 5, column: 9, scope: !39)
!39 = distinct !DILexicalBlock(scope: !40, file: !1, line: 5, column: 9)
!40 = distinct !DILexicalBlock(scope: !34, file: !1, line: 4, column: 23)
!41 = !DILocation(line: 5, column: 9, scope: !40)
!42 = !DILocation(line: 6, column: 16, scope: !39)
!43 = !DILocation(line: 6, column: 7, scope: !39)
!44 = !DILocation(line: 6, column: 12, scope: !39)
!45 = !DILocation(line: 8, column: 16, scope: !39)
!46 = !DILocation(line: 8, column: 7, scope: !39)
!47 = !DILocation(line: 8, column: 12, scope: !39)
!48 = !DILocation(line: 9, column: 3, scope: !40)
!49 = !DILocation(line: 4, column: 19, scope: !50)
!50 = !DILexicalBlockFile(scope: !34, file: !1, discriminator: 2)
!51 = !DILocation(line: 4, column: 3, scope: !50)
!52 = distinct !{!52, !53, !54}
!53 = !DILocation(line: 4, column: 3, scope: !35)
!54 = !DILocation(line: 9, column: 3, scope: !35)
!55 = !DILocation(line: 10, column: 3, scope: !7)
