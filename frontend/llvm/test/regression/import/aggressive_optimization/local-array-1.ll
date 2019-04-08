; ModuleID = 'local-array-1.pp.bc'
source_filename = "local-array-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !8 {
  %3 = alloca [10 x i32], align 16
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.declare(metadata [10 x i32]* %3, metadata !19, metadata !DIExpression()), !dbg !23
  call void @llvm.dbg.value(metadata i32 0, metadata !24, metadata !DIExpression()), !dbg !25
  br label %4, !dbg !26

; <label>:4:                                      ; preds = %6, %2
  %.0 = phi i32 [ 0, %2 ], [ %9, %6 ], !dbg !28
  call void @llvm.dbg.value(metadata i32 %.0, metadata !24, metadata !DIExpression()), !dbg !25
  %5 = icmp ult i32 %.0, 10, !dbg !29
  br i1 %5, label %6, label %10, !dbg !31

; <label>:6:                                      ; preds = %4
  %7 = zext i32 %.0 to i64, !dbg !32
  %8 = getelementptr inbounds [10 x i32], [10 x i32]* %3, i64 0, i64 %7, !dbg !32
  store i32 %.0, i32* %8, align 4, !dbg !34
  %9 = add nuw nsw i32 %.0, 1, !dbg !35
  call void @llvm.dbg.value(metadata i32 %9, metadata !24, metadata !DIExpression()), !dbg !25
  br label %4, !dbg !36, !llvm.loop !37

; <label>:10:                                     ; preds = %4
  call void @llvm.dbg.value(metadata !2, metadata !24, metadata !DIExpression()), !dbg !25
  ret i32 0, !dbg !39
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   [10 x si32]* $3 = allocate [10 x si32], 1, align 16
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #3} successors={#3, #4} {
; CHECK:   ui32 %4 = bitcast %.0
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#2} {
; CHECK:   %4 uilt 10
; CHECK:   ui32 %5 = bitcast %.0
; CHECK:   ui64 %6 = zext %5
; CHECK:   si32* %7 = ptrshift $3, 40 * 0, 4 * %6
; CHECK:   store %7, %.0, align 4
; CHECK:   si32 %8 = %.0 sadd.nw 1
; CHECK:   si32 %.0 = %8
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %4 uige 10
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 (tags/RELEASE_800/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "local-array-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 8.0.0 (tags/RELEASE_800/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !9, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!15 = !DILocalVariable(name: "argc", arg: 1, scope: !8, file: !1, line: 6, type: !11)
!16 = !DILocation(line: 6, column: 14, scope: !8)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !8, file: !1, line: 6, type: !12)
!18 = !DILocation(line: 6, column: 27, scope: !8)
!19 = !DILocalVariable(name: "a", scope: !8, file: !1, line: 8, type: !20)
!20 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 320, elements: !21)
!21 = !{!22}
!22 = !DISubrange(count: 10)
!23 = !DILocation(line: 8, column: 7, scope: !8)
!24 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 7, type: !11)
!25 = !DILocation(line: 7, column: 7, scope: !8)
!26 = !DILocation(line: 9, column: 8, scope: !27)
!27 = distinct !DILexicalBlock(scope: !8, file: !1, line: 9, column: 3)
!28 = !DILocation(line: 0, scope: !27)
!29 = !DILocation(line: 9, column: 17, scope: !30)
!30 = distinct !DILexicalBlock(scope: !27, file: !1, line: 9, column: 3)
!31 = !DILocation(line: 9, column: 3, scope: !27)
!32 = !DILocation(line: 10, column: 5, scope: !33)
!33 = distinct !DILexicalBlock(scope: !30, file: !1, line: 9, column: 28)
!34 = !DILocation(line: 10, column: 10, scope: !33)
!35 = !DILocation(line: 9, column: 24, scope: !30)
!36 = !DILocation(line: 9, column: 3, scope: !30)
!37 = distinct !{!37, !31, !38}
!38 = !DILocation(line: 11, column: 3, scope: !27)
!39 = !DILocation(line: 13, column: 3, scope: !8)
