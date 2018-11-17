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

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !8 {
  %3 = alloca %struct.foo, align 4
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.value(metadata i32 0, metadata !19, metadata !DIExpression()), !dbg !20
  %4 = getelementptr inbounds %struct.foo, %struct.foo* %3, i64 0, i32 1, i32 0
  %.promoted = load i32, i32* %4, align 4, !dbg !21
  br label %5, !dbg !31

; <label>:5:                                      ; preds = %18, %2
  %.lcssa6.lcssa7 = phi i32 [ %.promoted, %2 ], [ %.lcssa6.lcssa, %18 ]
  %.02 = phi i32 [ 0, %2 ], [ %19, %18 ], !dbg !32
  call void @llvm.dbg.value(metadata i32 %.02, metadata !19, metadata !DIExpression()), !dbg !20
  %6 = icmp ult i32 %.02, 10, !dbg !33
  br i1 %6, label %.preheader4.preheader, label %.preheader.preheader, !dbg !34

.preheader4.preheader:                            ; preds = %5
  %7 = zext i32 %.02 to i64
  br label %.preheader4, !dbg !35

.preheader.preheader:                             ; preds = %5
  %.lcssa6.lcssa7.lcssa = phi i32 [ %.lcssa6.lcssa7, %5 ]
  %.1.ph = phi i32 [ 0, %5 ]
  store i32 %.lcssa6.lcssa7.lcssa, i32* %4, align 4, !dbg !21
  br label %.preheader, !dbg !36

.preheader4:                                      ; preds = %.preheader4.preheader, %16
  %.lcssa6 = phi i32 [ %.lcssa, %16 ], [ %.lcssa6.lcssa7, %.preheader4.preheader ]
  %.01 = phi i32 [ %17, %16 ], [ 0, %.preheader4.preheader ], !dbg !39
  call void @llvm.dbg.value(metadata i32 %.01, metadata !40, metadata !DIExpression()), !dbg !41
  %8 = icmp ult i32 %.01, 10, !dbg !35
  br i1 %8, label %.preheader3.preheader, label %18, !dbg !42

.preheader3.preheader:                            ; preds = %.preheader4
  %9 = zext i32 %.01 to i64
  br label %.preheader3, !dbg !43

.preheader3:                                      ; preds = %.preheader3.preheader, %12
  %10 = phi i32 [ %0, %12 ], [ %.lcssa6, %.preheader3.preheader ]
  %.0 = phi i32 [ %15, %12 ], [ 0, %.preheader3.preheader ], !dbg !44
  call void @llvm.dbg.value(metadata i32 %.0, metadata !45, metadata !DIExpression()), !dbg !46
  %11 = icmp ult i32 %.0, 9, !dbg !43
  br i1 %11, label %12, label %16, !dbg !47

; <label>:12:                                     ; preds = %.preheader3
  %13 = zext i32 %.0 to i64, !dbg !48
  %14 = getelementptr inbounds %struct.foo, %struct.foo* %3, i64 0, i32 2, i64 %7, i64 %9, i64 %13, !dbg !48
  store i32 %0, i32* %14, align 4, !dbg !49
  %15 = add nuw nsw i32 %.0, 1, !dbg !50
  call void @llvm.dbg.value(metadata i32 %15, metadata !45, metadata !DIExpression()), !dbg !46
  br label %.preheader3, !dbg !51, !llvm.loop !52

; <label>:16:                                     ; preds = %.preheader3
  %.lcssa = phi i32 [ %10, %.preheader3 ]
  %17 = add nuw nsw i32 %.01, 1, !dbg !54
  call void @llvm.dbg.value(metadata i32 %17, metadata !40, metadata !DIExpression()), !dbg !41
  br label %.preheader4, !dbg !55, !llvm.loop !56

; <label>:18:                                     ; preds = %.preheader4
  %.lcssa6.lcssa = phi i32 [ %.lcssa6, %.preheader4 ]
  %19 = add nuw nsw i32 %.02, 1, !dbg !58
  call void @llvm.dbg.value(metadata i32 %19, metadata !19, metadata !DIExpression()), !dbg !20
  br label %5, !dbg !59, !llvm.loop !60

.preheader:                                       ; preds = %.preheader.preheader, %21
  %.1 = phi i32 [ %22, %21 ], [ %.1.ph, %.preheader.preheader ], !dbg !62
  call void @llvm.dbg.value(metadata i32 %.1, metadata !19, metadata !DIExpression()), !dbg !20
  %20 = icmp ult i32 %.1, 10, !dbg !36
  br i1 %20, label %21, label %23, !dbg !63

; <label>:21:                                     ; preds = %.preheader
  %22 = add nuw nsw i32 %.1, 1, !dbg !64
  call void @llvm.dbg.value(metadata i32 %22, metadata !19, metadata !DIExpression()), !dbg !20
  br label %.preheader, !dbg !65, !llvm.loop !66

; <label>:23:                                     ; preds = %.preheader
  ret i32 0, !dbg !68
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}* $3 = allocate {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}, 1, align 4
; CHECK:   si32* %4 = ptrshift $3, 3612 * 0, 1 * 4, 1 * 0
; CHECK:   si32 %.promoted = load %4, align 4
; CHECK:   si32 %.lcssa6.lcssa7 = %.promoted
; CHECK:   si32 %.02 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #6} successors={#3, #4} {
; CHECK:   ui32 %5 = bitcast %.02
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#.preheader4} {
; CHECK:   %5 uilt 10
; CHECK:   ui32 %6 = bitcast %.02
; CHECK:   ui64 %7 = zext %6
; CHECK:   si32 %.lcssa6 = %.lcssa6.lcssa7
; CHECK:   si32 %.01 = 0
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#.preheader} {
; CHECK:   %5 uige 10
; CHECK:   si32 %.lcssa6.lcssa7.lcssa = %.lcssa6.lcssa7
; CHECK:   si32 %.1.ph = 0
; CHECK:   store %4, %.lcssa6.lcssa7.lcssa, align 4
; CHECK:   si32 %.1 = %.1.ph
; CHECK: }
; CHECK: #.preheader4 predecessors={#3, #10} successors={#5, #6} {
; CHECK:   ui32 %8 = bitcast %.01
; CHECK: }
; CHECK: #5 predecessors={#.preheader4} successors={#.preheader3} {
; CHECK:   %8 uilt 10
; CHECK:   ui32 %10 = bitcast %.01
; CHECK:   ui64 %11 = zext %10
; CHECK:   si32 %12 = %.lcssa6
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #6 predecessors={#.preheader4} successors={#2} {
; CHECK:   %8 uige 10
; CHECK:   si32 %.lcssa6.lcssa = %.lcssa6
; CHECK:   si32 %13 = %.02 sadd.nw 1
; CHECK:   si32 %.lcssa6.lcssa7 = %.lcssa6.lcssa
; CHECK:   si32 %.02 = %13
; CHECK: }
; CHECK: #.preheader predecessors={#4, #7} successors={#7, #8} {
; CHECK:   ui32 %9 = bitcast %.1
; CHECK: }
; CHECK: #7 predecessors={#.preheader} successors={#.preheader} {
; CHECK:   %9 uilt 10
; CHECK:   si32 %14 = %.1 sadd.nw 1
; CHECK:   si32 %.1 = %14
; CHECK: }
; CHECK: #8 !exit predecessors={#.preheader} {
; CHECK:   %9 uige 10
; CHECK:   return 0
; CHECK: }
; CHECK: #.preheader3 predecessors={#5, #9} successors={#9, #10} {
; CHECK:   ui32 %15 = bitcast %.0
; CHECK: }
; CHECK: #9 predecessors={#.preheader3} successors={#.preheader3} {
; CHECK:   %15 uilt 9
; CHECK:   ui32 %16 = bitcast %.0
; CHECK:   ui64 %17 = zext %16
; CHECK:   si32* %18 = ptrshift $3, 3612 * 0, 1 * 12, 360 * %7, 36 * %11, 4 * %17
; CHECK:   store %18, %1, align 4
; CHECK:   si32 %19 = %.0 sadd.nw 1
; CHECK:   si32 %12 = %1
; CHECK:   si32 %.0 = %19
; CHECK: }
; CHECK: #10 predecessors={#.preheader3} successors={#.preheader4} {
; CHECK:   %15 uige 9
; CHECK:   si32 %.lcssa = %12
; CHECK:   si32 %20 = %.01 sadd.nw 1
; CHECK:   si32 %.lcssa6 = %.lcssa
; CHECK:   si32 %.01 = %20
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 7.0.0 (tags/RELEASE_700/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "phi-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 7.0.0 (tags/RELEASE_700/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 18, type: !9, isLocal: false, isDefinition: true, scopeLine: 18, flags: DIFlagPrototyped, isOptimized: false, unit: !0, retainedNodes: !2)
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
!19 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 19, type: !11)
!20 = !DILocation(line: 19, column: 7, scope: !8)
!21 = !DILocation(line: 25, column: 15, scope: !22)
!22 = distinct !DILexicalBlock(scope: !23, file: !1, line: 23, column: 43)
!23 = distinct !DILexicalBlock(scope: !24, file: !1, line: 23, column: 7)
!24 = distinct !DILexicalBlock(scope: !25, file: !1, line: 23, column: 7)
!25 = distinct !DILexicalBlock(scope: !26, file: !1, line: 22, column: 37)
!26 = distinct !DILexicalBlock(scope: !27, file: !1, line: 22, column: 5)
!27 = distinct !DILexicalBlock(scope: !28, file: !1, line: 22, column: 5)
!28 = distinct !DILexicalBlock(scope: !29, file: !1, line: 21, column: 35)
!29 = distinct !DILexicalBlock(scope: !30, file: !1, line: 21, column: 3)
!30 = distinct !DILexicalBlock(scope: !8, file: !1, line: 21, column: 3)
!31 = !DILocation(line: 21, column: 8, scope: !30)
!32 = !DILocation(line: 0, scope: !29)
!33 = !DILocation(line: 21, column: 17, scope: !29)
!34 = !DILocation(line: 21, column: 3, scope: !30)
!35 = !DILocation(line: 22, column: 19, scope: !26)
!36 = !DILocation(line: 30, column: 17, scope: !37)
!37 = distinct !DILexicalBlock(scope: !38, file: !1, line: 30, column: 3)
!38 = distinct !DILexicalBlock(scope: !8, file: !1, line: 30, column: 3)
!39 = !DILocation(line: 0, scope: !26)
!40 = !DILocalVariable(name: "j", scope: !8, file: !1, line: 19, type: !11)
!41 = !DILocation(line: 19, column: 10, scope: !8)
!42 = !DILocation(line: 22, column: 5, scope: !27)
!43 = !DILocation(line: 23, column: 21, scope: !23)
!44 = !DILocation(line: 0, scope: !23)
!45 = !DILocalVariable(name: "k", scope: !8, file: !1, line: 19, type: !11)
!46 = !DILocation(line: 19, column: 13, scope: !8)
!47 = !DILocation(line: 23, column: 7, scope: !24)
!48 = !DILocation(line: 24, column: 9, scope: !22)
!49 = !DILocation(line: 24, column: 22, scope: !22)
!50 = !DILocation(line: 23, column: 39, scope: !23)
!51 = !DILocation(line: 23, column: 7, scope: !23)
!52 = distinct !{!52, !47, !53}
!53 = !DILocation(line: 26, column: 7, scope: !24)
!54 = !DILocation(line: 22, column: 33, scope: !26)
!55 = !DILocation(line: 22, column: 5, scope: !26)
!56 = distinct !{!56, !42, !57}
!57 = !DILocation(line: 27, column: 5, scope: !27)
!58 = !DILocation(line: 21, column: 31, scope: !29)
!59 = !DILocation(line: 21, column: 3, scope: !29)
!60 = distinct !{!60, !34, !61}
!61 = !DILocation(line: 28, column: 3, scope: !30)
!62 = !DILocation(line: 0, scope: !37)
!63 = !DILocation(line: 30, column: 3, scope: !38)
!64 = !DILocation(line: 30, column: 31, scope: !37)
!65 = !DILocation(line: 30, column: 3, scope: !37)
!66 = distinct !{!66, !63, !67}
!67 = !DILocation(line: 32, column: 3, scope: !38)
!68 = !DILocation(line: 34, column: 3, scope: !8)
