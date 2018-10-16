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

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !7 {
  %3 = alloca %struct.foo, align 4
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !14, metadata !15), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !17, metadata !15), !dbg !18
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !19, metadata !15), !dbg !20
  br label %4, !dbg !21

; <label>:4:                                      ; preds = %17, %2
  %.02 = phi i32 [ 0, %2 ], [ %18, %17 ]
  call void @llvm.dbg.value(metadata i32 %.02, i64 0, metadata !19, metadata !15), !dbg !20
  %5 = icmp slt i32 %.02, 10, !dbg !23
  br i1 %5, label %.preheader4, label %.preheader, !dbg !26

.preheader4:                                      ; preds = %4, %15
  %.01 = phi i32 [ %16, %15 ], [ 0, %4 ]
  call void @llvm.dbg.value(metadata i32 %.01, i64 0, metadata !28, metadata !15), !dbg !29
  %6 = icmp slt i32 %.01, 10, !dbg !30
  br i1 %6, label %.preheader3, label %17, !dbg !35

.preheader3:                                      ; preds = %.preheader4, %8
  %.0 = phi i32 [ %14, %8 ], [ 0, %.preheader4 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !37, metadata !15), !dbg !38
  %7 = icmp slt i32 %.0, 9, !dbg !39
  br i1 %7, label %8, label %15, !dbg !44

; <label>:8:                                      ; preds = %.preheader3
  %9 = sext i32 %.02 to i64, !dbg !46
  %10 = sext i32 %.01 to i64, !dbg !46
  %11 = sext i32 %.0 to i64, !dbg !46
  %12 = getelementptr inbounds %struct.foo, %struct.foo* %3, i64 0, i32 2, i64 %9, i64 %10, i64 %11, !dbg !46
  store i32 %0, i32* %12, align 4, !dbg !48
  %13 = getelementptr inbounds %struct.foo, %struct.foo* %3, i64 0, i32 1, i32 0, !dbg !49
  store i32 %0, i32* %13, align 4, !dbg !50
  %14 = add nsw i32 %.0, 1, !dbg !51
  call void @llvm.dbg.value(metadata i32 %14, i64 0, metadata !37, metadata !15), !dbg !38
  br label %.preheader3, !dbg !53, !llvm.loop !54

; <label>:15:                                     ; preds = %.preheader3
  %16 = add nsw i32 %.01, 1, !dbg !57
  call void @llvm.dbg.value(metadata i32 %16, i64 0, metadata !28, metadata !15), !dbg !29
  br label %.preheader4, !dbg !59, !llvm.loop !60

; <label>:17:                                     ; preds = %.preheader4
  %18 = add nsw i32 %.02, 1, !dbg !63
  call void @llvm.dbg.value(metadata i32 %18, i64 0, metadata !19, metadata !15), !dbg !20
  br label %4, !dbg !65, !llvm.loop !66

.preheader:                                       ; preds = %4, %20
  %.1 = phi i32 [ %21, %20 ], [ 0, %4 ]
  call void @llvm.dbg.value(metadata i32 %.1, i64 0, metadata !19, metadata !15), !dbg !20
  %19 = icmp slt i32 %.1, 10, !dbg !69
  br i1 %19, label %20, label %22, !dbg !73

; <label>:20:                                     ; preds = %.preheader
  %21 = add nsw i32 %.1, 1, !dbg !75
  call void @llvm.dbg.value(metadata i32 %21, i64 0, metadata !19, metadata !15), !dbg !20
  br label %.preheader, !dbg !77, !llvm.loop !78

; <label>:22:                                     ; preds = %.preheader
  ret i32 0, !dbg !81
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}* $3 = allocate {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}, 1, align 4
; CHECK:   si32 %.02 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #6} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#.preheader4} {
; CHECK:   %.02 silt 10
; CHECK:   si32 %.01 = 0
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#.preheader} {
; CHECK:   %.02 sige 10
; CHECK:   si32 %.1 = 0
; CHECK: }
; CHECK: #.preheader4 predecessors={#3, #10} successors={#5, #6} {
; CHECK: }
; CHECK: #5 predecessors={#.preheader4} successors={#.preheader3} {
; CHECK:   %.01 silt 10
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #6 predecessors={#.preheader4} successors={#2} {
; CHECK:   %.01 sige 10
; CHECK:   si32 %4 = %.02 sadd.nw 1
; CHECK:   si32 %.02 = %4
; CHECK: }
; CHECK: #.preheader predecessors={#4, #7} successors={#7, #8} {
; CHECK: }
; CHECK: #7 predecessors={#.preheader} successors={#.preheader} {
; CHECK:   %.1 silt 10
; CHECK:   si32 %5 = %.1 sadd.nw 1
; CHECK:   si32 %.1 = %5
; CHECK: }
; CHECK: #8 !exit predecessors={#.preheader} {
; CHECK:   %.1 sige 10
; CHECK:   return 0
; CHECK: }
; CHECK: #.preheader3 predecessors={#5, #9} successors={#9, #10} {
; CHECK: }
; CHECK: #9 predecessors={#.preheader3} successors={#.preheader3} {
; CHECK:   %.0 silt 9
; CHECK:   si64 %6 = sext %.02
; CHECK:   si64 %7 = sext %.01
; CHECK:   si64 %8 = sext %.0
; CHECK:   si32* %9 = ptrshift $3, 3612 * 0, 1 * 12, 360 * %6, 36 * %7, 4 * %8
; CHECK:   store %9, %1, align 4
; CHECK:   si32* %10 = ptrshift $3, 3612 * 0, 1 * 4, 1 * 0
; CHECK:   store %10, %1, align 4
; CHECK:   si32 %11 = %.0 sadd.nw 1
; CHECK:   si32 %.0 = %11
; CHECK: }
; CHECK: #10 predecessors={#.preheader3} successors={#.preheader4} {
; CHECK:   %.0 sige 9
; CHECK:   si32 %12 = %.01 sadd.nw 1
; CHECK:   si32 %.01 = %12
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "phi-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
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
!21 = !DILocation(line: 21, column: 8, scope: !22)
!22 = distinct !DILexicalBlock(scope: !7, file: !1, line: 21, column: 3)
!23 = !DILocation(line: 21, column: 17, scope: !24)
!24 = !DILexicalBlockFile(scope: !25, file: !1, discriminator: 1)
!25 = distinct !DILexicalBlock(scope: !22, file: !1, line: 21, column: 3)
!26 = !DILocation(line: 21, column: 3, scope: !27)
!27 = !DILexicalBlockFile(scope: !22, file: !1, discriminator: 1)
!28 = !DILocalVariable(name: "j", scope: !7, file: !1, line: 19, type: !10)
!29 = !DILocation(line: 19, column: 10, scope: !7)
!30 = !DILocation(line: 22, column: 19, scope: !31)
!31 = !DILexicalBlockFile(scope: !32, file: !1, discriminator: 1)
!32 = distinct !DILexicalBlock(scope: !33, file: !1, line: 22, column: 5)
!33 = distinct !DILexicalBlock(scope: !34, file: !1, line: 22, column: 5)
!34 = distinct !DILexicalBlock(scope: !25, file: !1, line: 21, column: 35)
!35 = !DILocation(line: 22, column: 5, scope: !36)
!36 = !DILexicalBlockFile(scope: !33, file: !1, discriminator: 1)
!37 = !DILocalVariable(name: "k", scope: !7, file: !1, line: 19, type: !10)
!38 = !DILocation(line: 19, column: 13, scope: !7)
!39 = !DILocation(line: 23, column: 21, scope: !40)
!40 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 1)
!41 = distinct !DILexicalBlock(scope: !42, file: !1, line: 23, column: 7)
!42 = distinct !DILexicalBlock(scope: !43, file: !1, line: 23, column: 7)
!43 = distinct !DILexicalBlock(scope: !32, file: !1, line: 22, column: 37)
!44 = !DILocation(line: 23, column: 7, scope: !45)
!45 = !DILexicalBlockFile(scope: !42, file: !1, discriminator: 1)
!46 = !DILocation(line: 24, column: 9, scope: !47)
!47 = distinct !DILexicalBlock(scope: !41, file: !1, line: 23, column: 43)
!48 = !DILocation(line: 24, column: 22, scope: !47)
!49 = !DILocation(line: 25, column: 13, scope: !47)
!50 = !DILocation(line: 25, column: 15, scope: !47)
!51 = !DILocation(line: 23, column: 39, scope: !52)
!52 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 2)
!53 = !DILocation(line: 23, column: 7, scope: !52)
!54 = distinct !{!54, !55, !56}
!55 = !DILocation(line: 23, column: 7, scope: !42)
!56 = !DILocation(line: 26, column: 7, scope: !42)
!57 = !DILocation(line: 22, column: 33, scope: !58)
!58 = !DILexicalBlockFile(scope: !32, file: !1, discriminator: 2)
!59 = !DILocation(line: 22, column: 5, scope: !58)
!60 = distinct !{!60, !61, !62}
!61 = !DILocation(line: 22, column: 5, scope: !33)
!62 = !DILocation(line: 27, column: 5, scope: !33)
!63 = !DILocation(line: 21, column: 31, scope: !64)
!64 = !DILexicalBlockFile(scope: !25, file: !1, discriminator: 2)
!65 = !DILocation(line: 21, column: 3, scope: !64)
!66 = distinct !{!66, !67, !68}
!67 = !DILocation(line: 21, column: 3, scope: !22)
!68 = !DILocation(line: 28, column: 3, scope: !22)
!69 = !DILocation(line: 30, column: 17, scope: !70)
!70 = !DILexicalBlockFile(scope: !71, file: !1, discriminator: 1)
!71 = distinct !DILexicalBlock(scope: !72, file: !1, line: 30, column: 3)
!72 = distinct !DILexicalBlock(scope: !7, file: !1, line: 30, column: 3)
!73 = !DILocation(line: 30, column: 3, scope: !74)
!74 = !DILexicalBlockFile(scope: !72, file: !1, discriminator: 1)
!75 = !DILocation(line: 30, column: 31, scope: !76)
!76 = !DILexicalBlockFile(scope: !71, file: !1, discriminator: 2)
!77 = !DILocation(line: 30, column: 3, scope: !76)
!78 = distinct !{!78, !79, !80}
!79 = !DILocation(line: 30, column: 3, scope: !72)
!80 = !DILocation(line: 32, column: 3, scope: !72)
!81 = !DILocation(line: 34, column: 3, scope: !7)
