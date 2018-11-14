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
  br label %4, !dbg !21

; <label>:4:                                      ; preds = %21, %2
  %.02 = phi i32 [ 0, %2 ], [ %22, %21 ]
  call void @llvm.dbg.value(metadata i32 %.02, metadata !19, metadata !DIExpression()), !dbg !20
  %5 = icmp ult i32 %.02, 10, !dbg !23
  br i1 %5, label %6, label %23, !dbg !25

; <label>:6:                                      ; preds = %4
  call void @llvm.dbg.value(metadata i32 0, metadata !26, metadata !DIExpression()), !dbg !27
  br label %7, !dbg !28

; <label>:7:                                      ; preds = %19, %6
  %.01 = phi i32 [ 0, %6 ], [ %20, %19 ]
  call void @llvm.dbg.value(metadata i32 %.01, metadata !26, metadata !DIExpression()), !dbg !27
  %8 = icmp ult i32 %.01, 10, !dbg !31
  br i1 %8, label %9, label %21, !dbg !33

; <label>:9:                                      ; preds = %7
  call void @llvm.dbg.value(metadata i32 0, metadata !34, metadata !DIExpression()), !dbg !35
  br label %10, !dbg !36

; <label>:10:                                     ; preds = %12, %9
  %.0 = phi i32 [ 0, %9 ], [ %18, %12 ]
  call void @llvm.dbg.value(metadata i32 %.0, metadata !34, metadata !DIExpression()), !dbg !35
  %11 = icmp ult i32 %.0, 9, !dbg !39
  br i1 %11, label %12, label %19, !dbg !41

; <label>:12:                                     ; preds = %10
  %13 = zext i32 %.02 to i64, !dbg !42
  %14 = zext i32 %.01 to i64, !dbg !42
  %15 = zext i32 %.0 to i64, !dbg !42
  %16 = getelementptr inbounds %struct.foo, %struct.foo* %3, i64 0, i32 2, i64 %13, i64 %14, i64 %15, !dbg !42
  store i32 %0, i32* %16, align 4, !dbg !44
  %17 = getelementptr inbounds %struct.foo, %struct.foo* %3, i64 0, i32 1, i32 0, !dbg !45
  store i32 %0, i32* %17, align 4, !dbg !46
  %18 = add nuw nsw i32 %.0, 1, !dbg !47
  call void @llvm.dbg.value(metadata i32 %18, metadata !34, metadata !DIExpression()), !dbg !35
  br label %10, !dbg !48, !llvm.loop !49

; <label>:19:                                     ; preds = %10
  %20 = add nuw nsw i32 %.01, 1, !dbg !51
  call void @llvm.dbg.value(metadata i32 %20, metadata !26, metadata !DIExpression()), !dbg !27
  br label %7, !dbg !52, !llvm.loop !53

; <label>:21:                                     ; preds = %7
  %22 = add nuw nsw i32 %.02, 1, !dbg !55
  call void @llvm.dbg.value(metadata i32 %22, metadata !19, metadata !DIExpression()), !dbg !20
  br label %4, !dbg !56, !llvm.loop !57

; <label>:23:                                     ; preds = %4
  call void @llvm.dbg.value(metadata i32 0, metadata !19, metadata !DIExpression()), !dbg !20
  br label %24, !dbg !59

; <label>:24:                                     ; preds = %26, %23
  %.1 = phi i32 [ 0, %23 ], [ %27, %26 ]
  call void @llvm.dbg.value(metadata i32 %.1, metadata !19, metadata !DIExpression()), !dbg !20
  %25 = icmp ult i32 %.1, 10, !dbg !61
  br i1 %25, label %26, label %28, !dbg !63

; <label>:26:                                     ; preds = %24
  %27 = add nuw nsw i32 %.1, 1, !dbg !64
  call void @llvm.dbg.value(metadata i32 %27, metadata !19, metadata !DIExpression()), !dbg !20
  br label %24, !dbg !65, !llvm.loop !66

; <label>:28:                                     ; preds = %24
  ret i32 0, !dbg !68
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}* $3 = allocate {0: si32, 4: {0: si32, 4: float}, 12: [10 x [10 x [9 x si32]]]}, 1, align 4
; CHECK:   si32 %.02 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #8} successors={#3, #4} {
; CHECK:   ui32 %4 = bitcast %.02
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#5} {
; CHECK:   %4 uilt 10
; CHECK:   si32 %.01 = 0
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#6} {
; CHECK:   %4 uige 10
; CHECK:   si32 %.1 = 0
; CHECK: }
; CHECK: #5 predecessors={#3, #13} successors={#7, #8} {
; CHECK:   ui32 %5 = bitcast %.01
; CHECK: }
; CHECK: #7 predecessors={#5} successors={#11} {
; CHECK:   %5 uilt 10
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #8 predecessors={#5} successors={#2} {
; CHECK:   %5 uige 10
; CHECK:   si32 %7 = %.02 sadd.nw 1
; CHECK:   si32 %.02 = %7
; CHECK: }
; CHECK: #6 predecessors={#4, #9} successors={#9, #10} {
; CHECK:   ui32 %6 = bitcast %.1
; CHECK: }
; CHECK: #9 predecessors={#6} successors={#6} {
; CHECK:   %6 uilt 10
; CHECK:   si32 %8 = %.1 sadd.nw 1
; CHECK:   si32 %.1 = %8
; CHECK: }
; CHECK: #10 !exit predecessors={#6} {
; CHECK:   %6 uige 10
; CHECK:   return 0
; CHECK: }
; CHECK: #11 predecessors={#7, #12} successors={#12, #13} {
; CHECK:   ui32 %9 = bitcast %.0
; CHECK: }
; CHECK: #12 predecessors={#11} successors={#11} {
; CHECK:   %9 uilt 9
; CHECK:   ui32 %10 = bitcast %.02
; CHECK:   ui64 %11 = zext %10
; CHECK:   ui32 %12 = bitcast %.01
; CHECK:   ui64 %13 = zext %12
; CHECK:   ui32 %14 = bitcast %.0
; CHECK:   ui64 %15 = zext %14
; CHECK:   si32* %16 = ptrshift $3, 3612 * 0, 1 * 12, 360 * %11, 36 * %13, 4 * %15
; CHECK:   store %16, %1, align 4
; CHECK:   si32* %17 = ptrshift $3, 3612 * 0, 1 * 4, 1 * 0
; CHECK:   store %17, %1, align 4
; CHECK:   si32 %18 = %.0 sadd.nw 1
; CHECK:   si32 %.0 = %18
; CHECK: }
; CHECK: #13 predecessors={#11} successors={#5} {
; CHECK:   %9 uige 9
; CHECK:   si32 %19 = %.01 sadd.nw 1
; CHECK:   si32 %.01 = %19
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 6.0.1 (tags/RELEASE_601/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "phi-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
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
!19 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 19, type: !11)
!20 = !DILocation(line: 19, column: 7, scope: !8)
!21 = !DILocation(line: 21, column: 8, scope: !22)
!22 = distinct !DILexicalBlock(scope: !8, file: !1, line: 21, column: 3)
!23 = !DILocation(line: 21, column: 17, scope: !24)
!24 = distinct !DILexicalBlock(scope: !22, file: !1, line: 21, column: 3)
!25 = !DILocation(line: 21, column: 3, scope: !22)
!26 = !DILocalVariable(name: "j", scope: !8, file: !1, line: 19, type: !11)
!27 = !DILocation(line: 19, column: 10, scope: !8)
!28 = !DILocation(line: 22, column: 10, scope: !29)
!29 = distinct !DILexicalBlock(scope: !30, file: !1, line: 22, column: 5)
!30 = distinct !DILexicalBlock(scope: !24, file: !1, line: 21, column: 35)
!31 = !DILocation(line: 22, column: 19, scope: !32)
!32 = distinct !DILexicalBlock(scope: !29, file: !1, line: 22, column: 5)
!33 = !DILocation(line: 22, column: 5, scope: !29)
!34 = !DILocalVariable(name: "k", scope: !8, file: !1, line: 19, type: !11)
!35 = !DILocation(line: 19, column: 13, scope: !8)
!36 = !DILocation(line: 23, column: 12, scope: !37)
!37 = distinct !DILexicalBlock(scope: !38, file: !1, line: 23, column: 7)
!38 = distinct !DILexicalBlock(scope: !32, file: !1, line: 22, column: 37)
!39 = !DILocation(line: 23, column: 21, scope: !40)
!40 = distinct !DILexicalBlock(scope: !37, file: !1, line: 23, column: 7)
!41 = !DILocation(line: 23, column: 7, scope: !37)
!42 = !DILocation(line: 24, column: 9, scope: !43)
!43 = distinct !DILexicalBlock(scope: !40, file: !1, line: 23, column: 43)
!44 = !DILocation(line: 24, column: 22, scope: !43)
!45 = !DILocation(line: 25, column: 13, scope: !43)
!46 = !DILocation(line: 25, column: 15, scope: !43)
!47 = !DILocation(line: 23, column: 39, scope: !40)
!48 = !DILocation(line: 23, column: 7, scope: !40)
!49 = distinct !{!49, !41, !50}
!50 = !DILocation(line: 26, column: 7, scope: !37)
!51 = !DILocation(line: 22, column: 33, scope: !32)
!52 = !DILocation(line: 22, column: 5, scope: !32)
!53 = distinct !{!53, !33, !54}
!54 = !DILocation(line: 27, column: 5, scope: !29)
!55 = !DILocation(line: 21, column: 31, scope: !24)
!56 = !DILocation(line: 21, column: 3, scope: !24)
!57 = distinct !{!57, !25, !58}
!58 = !DILocation(line: 28, column: 3, scope: !22)
!59 = !DILocation(line: 30, column: 8, scope: !60)
!60 = distinct !DILexicalBlock(scope: !8, file: !1, line: 30, column: 3)
!61 = !DILocation(line: 30, column: 17, scope: !62)
!62 = distinct !DILexicalBlock(scope: !60, file: !1, line: 30, column: 3)
!63 = !DILocation(line: 30, column: 3, scope: !60)
!64 = !DILocation(line: 30, column: 31, scope: !62)
!65 = !DILocation(line: 30, column: 3, scope: !62)
!66 = distinct !{!66, !63, !67}
!67 = !DILocation(line: 32, column: 3, scope: !60)
!68 = !DILocation(line: 34, column: 3, scope: !8)
