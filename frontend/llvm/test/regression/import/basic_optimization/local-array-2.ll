; ModuleID = 'local-array-2.pp.bc'
source_filename = "local-array-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

@.str = private unnamed_addr constant [34 x i8] c"This is string.h library function\00", align 1
; CHECK: define [34 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [84, 104, 105, 115, 32, 105, 115, 32, 115, 116, 114, 105, 110, 103, 46, 104, 32, 108, 105, 98, 114, 97, 114, 121, 32, 102, 117, 110, 99, 116, 105, 111, 110, 0], align 1
; CHECK: }
; CHECK: }

declare i32 @puts(i8*) #2
; CHECK: declare si32 @ar.libc.puts(si8*)

declare i8* @strcpy(i8*, i8*) #2
; CHECK: declare si8* @ar.libc.strcpy(si8*, si8*)

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #3
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #3
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i8* @foo(i8*, i32) #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i8* %0, metadata !14, metadata !DIExpression()), !dbg !15
  call void @llvm.dbg.value(metadata i32 %1, metadata !16, metadata !DIExpression()), !dbg !15
  call void @llvm.dbg.value(metadata i32 0, metadata !17, metadata !DIExpression()), !dbg !15
  br label %3, !dbg !18

3:                                                ; preds = %8, %2
  %.0 = phi i32 [ 0, %2 ], [ %9, %8 ], !dbg !20
  call void @llvm.dbg.value(metadata i32 %.0, metadata !17, metadata !DIExpression()), !dbg !15
  %4 = icmp slt i32 %.0, %1, !dbg !21
  br i1 %4, label %5, label %10, !dbg !23

5:                                                ; preds = %3
  %6 = sext i32 %.0 to i64, !dbg !24
  %7 = getelementptr inbounds i8, i8* %0, i64 %6, !dbg !24
  store i8 65, i8* %7, align 1, !dbg !25
  br label %8, !dbg !24

8:                                                ; preds = %5
  %9 = add nsw i32 %.0, 1, !dbg !26
  call void @llvm.dbg.value(metadata i32 %9, metadata !17, metadata !DIExpression()), !dbg !15
  br label %3, !dbg !27, !llvm.loop !28

10:                                               ; preds = %3
  ret i8* %0, !dbg !30
}
; CHECK: define si8* @foo(si8* %1, si32 %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #3} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#2} {
; CHECK:   %.0 silt %2
; CHECK:   si64 %3 = sext %.0
; CHECK:   si8* %4 = ptrshift %1, 1 * %3
; CHECK:   store %4, 65, align 1
; CHECK:   si32 %5 = %.0 sadd.nw 1
; CHECK:   si32 %.0 = %5
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %.0 sige %2
; CHECK:   return %1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !31 {
  %1 = alloca [50 x i8], align 16
  %2 = alloca [10 x i8], align 1
  call void @llvm.dbg.declare(metadata [50 x i8]* %1, metadata !34, metadata !DIExpression()), !dbg !38
  %3 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i64 0, i64 0, !dbg !39
  %4 = getelementptr inbounds [34 x i8], [34 x i8]* @.str, i64 0, i64 0, !dbg !40
  %5 = call i8* @strcpy(i8* %3, i8* %4), !dbg !40
  %6 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i64 0, i64 0, !dbg !41
  %7 = call i32 @puts(i8* %6), !dbg !42
  %8 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i64 0, i64 0, !dbg !43
  call void @llvm.memset.p0i8.i64(i8* align 16 %8, i8 36, i64 50, i1 false), !dbg !43
  %9 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i64 0, i64 0, !dbg !44
  %10 = call i8* @foo(i8* %9, i32 10), !dbg !45
  call void @llvm.dbg.value(metadata i8* %10, metadata !46, metadata !DIExpression()), !dbg !47
  call void @llvm.dbg.declare(metadata [10 x i8]* %2, metadata !48, metadata !DIExpression()), !dbg !52
  %11 = getelementptr inbounds [10 x i8], [10 x i8]* %2, i64 0, i64 0, !dbg !53
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %11, i8* align 1 %10, i64 10, i1 false), !dbg !53
  %12 = getelementptr inbounds [10 x i8], [10 x i8]* %2, i64 0, i64 0, !dbg !54
  %13 = call i8* @foo(i8* %12, i32 10), !dbg !55
  call void @llvm.dbg.value(metadata i8* %13, metadata !56, metadata !DIExpression()), !dbg !47
  %14 = call i32 @puts(i8* %13), !dbg !57
  ret i32 0, !dbg !58
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   [50 x si8]* $1 = allocate [50 x si8], 1, align 16
; CHECK:   [10 x si8]* $2 = allocate [10 x si8], 1, align 1
; CHECK:   si8* %3 = ptrshift $1, 50 * 0, 1 * 0
; CHECK:   si8* %4 = ptrshift @.str, 34 * 0, 1 * 0
; CHECK:   si8* %5 = call @ar.libc.strcpy(%3, %4)
; CHECK:   si8* %6 = ptrshift $1, 50 * 0, 1 * 0
; CHECK:   si32 %7 = call @ar.libc.puts(%6)
; CHECK:   si8* %8 = ptrshift $1, 50 * 0, 1 * 0
; CHECK:   call @ar.memset(%8, 36, 50, 16, 0)
; CHECK:   si8* %9 = ptrshift $1, 50 * 0, 1 * 0
; CHECK:   si8* %10 = call @foo(%9, 10)
; CHECK:   si8* %11 = ptrshift $2, 10 * 0, 1 * 0
; CHECK:   call @ar.memcpy(%11, %10, 10, 1, 1, 0)
; CHECK:   si8* %12 = ptrshift $2, 10 * 0, 1 * 0
; CHECK:   si8* %13 = call @foo(%12, 10)
; CHECK:   si32 %14 = call @ar.libc.puts(%13)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "local-array-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 4, type: !9, scopeLine: 4, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11, !13}
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !DILocalVariable(name: "a", arg: 1, scope: !8, file: !1, line: 4, type: !11)
!15 = !DILocation(line: 0, scope: !8)
!16 = !DILocalVariable(name: "n", arg: 2, scope: !8, file: !1, line: 4, type: !13)
!17 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 5, type: !13)
!18 = !DILocation(line: 6, column: 8, scope: !19)
!19 = distinct !DILexicalBlock(scope: !8, file: !1, line: 6, column: 3)
!20 = !DILocation(line: 0, scope: !19)
!21 = !DILocation(line: 6, column: 17, scope: !22)
!22 = distinct !DILexicalBlock(scope: !19, file: !1, line: 6, column: 3)
!23 = !DILocation(line: 6, column: 3, scope: !19)
!24 = !DILocation(line: 7, column: 5, scope: !22)
!25 = !DILocation(line: 7, column: 10, scope: !22)
!26 = !DILocation(line: 6, column: 23, scope: !22)
!27 = !DILocation(line: 6, column: 3, scope: !22)
!28 = distinct !{!28, !23, !29}
!29 = !DILocation(line: 7, column: 12, scope: !19)
!30 = !DILocation(line: 8, column: 3, scope: !8)
!31 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !32, scopeLine: 11, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!32 = !DISubroutineType(types: !33)
!33 = !{!13}
!34 = !DILocalVariable(name: "str", scope: !31, file: !1, line: 12, type: !35)
!35 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 400, elements: !36)
!36 = !{!37}
!37 = !DISubrange(count: 50)
!38 = !DILocation(line: 12, column: 8, scope: !31)
!39 = !DILocation(line: 14, column: 10, scope: !31)
!40 = !DILocation(line: 14, column: 3, scope: !31)
!41 = !DILocation(line: 15, column: 8, scope: !31)
!42 = !DILocation(line: 15, column: 3, scope: !31)
!43 = !DILocation(line: 17, column: 3, scope: !31)
!44 = !DILocation(line: 18, column: 17, scope: !31)
!45 = !DILocation(line: 18, column: 13, scope: !31)
!46 = !DILocalVariable(name: "A", scope: !31, file: !1, line: 18, type: !11)
!47 = !DILocation(line: 0, scope: !31)
!48 = !DILocalVariable(name: "B", scope: !31, file: !1, line: 20, type: !49)
!49 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 80, elements: !50)
!50 = !{!51}
!51 = !DISubrange(count: 10)
!52 = !DILocation(line: 20, column: 8, scope: !31)
!53 = !DILocation(line: 21, column: 3, scope: !31)
!54 = !DILocation(line: 22, column: 17, scope: !31)
!55 = !DILocation(line: 22, column: 13, scope: !31)
!56 = !DILocalVariable(name: "C", scope: !31, file: !1, line: 22, type: !11)
!57 = !DILocation(line: 23, column: 3, scope: !31)
!58 = !DILocation(line: 24, column: 3, scope: !31)
