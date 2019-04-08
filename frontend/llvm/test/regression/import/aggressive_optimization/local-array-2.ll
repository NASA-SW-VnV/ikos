; ModuleID = 'local-array-2.pp.bc'
source_filename = "local-array-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

@.str = private unnamed_addr constant [34 x i8] c"This is string.h library function\00", align 1
; CHECK: define [34 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [84, 104, 105, 115, 32, 105, 115, 32, 115, 116, 114, 105, 110, 103, 46, 104, 32, 108, 105, 98, 114, 97, 114, 121, 32, 102, 117, 110, 99, 116, 105, 111, 110, 0], align 1
; CHECK: }
; CHECK: }

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #2
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #2
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc i8* @foo(i8*, i32) unnamed_addr #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i8* %0, metadata !14, metadata !DIExpression()), !dbg !15
  call void @llvm.dbg.value(metadata i32 %1, metadata !16, metadata !DIExpression()), !dbg !17
  call void @llvm.dbg.value(metadata i32 0, metadata !18, metadata !DIExpression()), !dbg !19
  br label %3, !dbg !20

; <label>:3:                                      ; preds = %5, %2
  %.0 = phi i32 [ 0, %2 ], [ %8, %5 ], !dbg !22
  call void @llvm.dbg.value(metadata i32 %.0, metadata !18, metadata !DIExpression()), !dbg !19
  %4 = icmp slt i32 %.0, %1, !dbg !23
  br i1 %4, label %5, label %9, !dbg !25

; <label>:5:                                      ; preds = %3
  %6 = zext i32 %.0 to i64, !dbg !26
  %7 = getelementptr inbounds i8, i8* %0, i64 %6, !dbg !26
  store i8 65, i8* %7, align 1, !dbg !27
  %8 = add nuw nsw i32 %.0, 1, !dbg !28
  call void @llvm.dbg.value(metadata i32 %8, metadata !18, metadata !DIExpression()), !dbg !19
  br label %3, !dbg !29, !llvm.loop !30

; <label>:9:                                      ; preds = %3
  ret i8* %0, !dbg !32
}
; CHECK: define si8* @foo(si8* %1, si32 %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #3} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#2} {
; CHECK:   %.0 silt %2
; CHECK:   ui32 %3 = bitcast %.0
; CHECK:   ui64 %4 = zext %3
; CHECK:   si8* %5 = ptrshift %1, 1 * %4
; CHECK:   store %5, 65, align 1
; CHECK:   si32 %6 = %.0 sadd.nw 1
; CHECK:   si32 %.0 = %6
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %.0 sige %2
; CHECK:   return %1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() local_unnamed_addr #0 !dbg !33 {
  %1 = alloca [50 x i8], align 16
  %2 = alloca [10 x i8], align 1
  call void @llvm.dbg.declare(metadata [50 x i8]* %1, metadata !36, metadata !DIExpression()), !dbg !40
  %3 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i64 0, i64 0, !dbg !41
  %4 = getelementptr inbounds [34 x i8], [34 x i8]* @.str, i64 0, i64 0, !dbg !42
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 16 %3, i8* align 1 %4, i64 34, i1 false), !dbg !42
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 %3, i8 36, i64 50, i1 false), !dbg !43
  %5 = call fastcc i8* @foo(i8* nonnull %3, i32 10), !dbg !44
  call void @llvm.dbg.value(metadata i8* %5, metadata !45, metadata !DIExpression()), !dbg !46
  call void @llvm.dbg.declare(metadata [10 x i8]* %2, metadata !47, metadata !DIExpression()), !dbg !51
  %6 = getelementptr inbounds [10 x i8], [10 x i8]* %2, i64 0, i64 0, !dbg !52
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 1 %6, i8* align 1 %5, i64 10, i1 false), !dbg !52
  %7 = call fastcc i8* @foo(i8* nonnull %6, i32 10), !dbg !53
  call void @llvm.dbg.value(metadata i8* %7, metadata !54, metadata !DIExpression()), !dbg !55
  ret i32 0, !dbg !56
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   [50 x si8]* $1 = allocate [50 x si8], 1, align 16
; CHECK:   [10 x si8]* $2 = allocate [10 x si8], 1, align 1
; CHECK:   si8* %3 = ptrshift $1, 50 * 0, 1 * 0
; CHECK:   si8* %4 = ptrshift @.str, 34 * 0, 1 * 0
; CHECK:   call @ar.memcpy(%3, %4, 34, 16, 1, 0)
; CHECK:   call @ar.memset(%3, 36, 50, 16, 0)
; CHECK:   si8* %5 = call @foo(%3, 10)
; CHECK:   si8* %6 = ptrshift $2, 10 * 0, 1 * 0
; CHECK:   call @ar.memcpy(%6, %5, 10, 1, 1, 0)
; CHECK:   si8* %7 = call @foo(%6, 10)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 (tags/RELEASE_800/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "local-array-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 8.0.0 (tags/RELEASE_800/final)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 4, type: !9, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11, !13}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !DILocalVariable(name: "a", arg: 1, scope: !8, file: !1, line: 4, type: !11)
!15 = !DILocation(line: 4, column: 17, scope: !8)
!16 = !DILocalVariable(name: "n", arg: 2, scope: !8, file: !1, line: 4, type: !13)
!17 = !DILocation(line: 4, column: 24, scope: !8)
!18 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 5, type: !13)
!19 = !DILocation(line: 5, column: 7, scope: !8)
!20 = !DILocation(line: 6, column: 8, scope: !21)
!21 = distinct !DILexicalBlock(scope: !8, file: !1, line: 6, column: 3)
!22 = !DILocation(line: 0, scope: !21)
!23 = !DILocation(line: 6, column: 17, scope: !24)
!24 = distinct !DILexicalBlock(scope: !21, file: !1, line: 6, column: 3)
!25 = !DILocation(line: 6, column: 3, scope: !21)
!26 = !DILocation(line: 7, column: 5, scope: !24)
!27 = !DILocation(line: 7, column: 10, scope: !24)
!28 = !DILocation(line: 6, column: 23, scope: !24)
!29 = !DILocation(line: 6, column: 3, scope: !24)
!30 = distinct !{!30, !25, !31}
!31 = !DILocation(line: 7, column: 12, scope: !21)
!32 = !DILocation(line: 8, column: 3, scope: !8)
!33 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !34, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!34 = !DISubroutineType(types: !35)
!35 = !{!13}
!36 = !DILocalVariable(name: "str", scope: !33, file: !1, line: 12, type: !37)
!37 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 400, elements: !38)
!38 = !{!39}
!39 = !DISubrange(count: 50)
!40 = !DILocation(line: 12, column: 8, scope: !33)
!41 = !DILocation(line: 14, column: 10, scope: !33)
!42 = !DILocation(line: 14, column: 3, scope: !33)
!43 = !DILocation(line: 17, column: 3, scope: !33)
!44 = !DILocation(line: 18, column: 13, scope: !33)
!45 = !DILocalVariable(name: "A", scope: !33, file: !1, line: 18, type: !11)
!46 = !DILocation(line: 18, column: 9, scope: !33)
!47 = !DILocalVariable(name: "B", scope: !33, file: !1, line: 20, type: !48)
!48 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 80, elements: !49)
!49 = !{!50}
!50 = !DISubrange(count: 10)
!51 = !DILocation(line: 20, column: 8, scope: !33)
!52 = !DILocation(line: 21, column: 3, scope: !33)
!53 = !DILocation(line: 22, column: 13, scope: !33)
!54 = !DILocalVariable(name: "C", scope: !33, file: !1, line: 22, type: !11)
!55 = !DILocation(line: 22, column: 9, scope: !33)
!56 = !DILocation(line: 24, column: 3, scope: !33)
