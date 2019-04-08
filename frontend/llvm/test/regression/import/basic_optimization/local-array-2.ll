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

declare i32 @puts(i8*) #2
; CHECK: declare si32 @ar.libc.puts(si8*)

declare i8* @strcpy(i8*, i8*) #2
; CHECK: declare si8* @ar.libc.strcpy(si8*, si8*)

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #3
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #3
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i8* @foo(i8*, i32) #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i8* %0, metadata !14, metadata !DIExpression()), !dbg !15
  call void @llvm.dbg.value(metadata i32 %1, metadata !16, metadata !DIExpression()), !dbg !17
  call void @llvm.dbg.value(metadata i32 0, metadata !18, metadata !DIExpression()), !dbg !19
  br label %3, !dbg !20

; <label>:3:                                      ; preds = %8, %2
  %.0 = phi i32 [ 0, %2 ], [ %9, %8 ], !dbg !22
  call void @llvm.dbg.value(metadata i32 %.0, metadata !18, metadata !DIExpression()), !dbg !19
  %4 = icmp slt i32 %.0, %1, !dbg !23
  br i1 %4, label %5, label %10, !dbg !25

; <label>:5:                                      ; preds = %3
  %6 = sext i32 %.0 to i64, !dbg !26
  %7 = getelementptr inbounds i8, i8* %0, i64 %6, !dbg !26
  store i8 65, i8* %7, align 1, !dbg !27
  br label %8, !dbg !26

; <label>:8:                                      ; preds = %5
  %9 = add nsw i32 %.0, 1, !dbg !28
  call void @llvm.dbg.value(metadata i32 %9, metadata !18, metadata !DIExpression()), !dbg !19
  br label %3, !dbg !29, !llvm.loop !30

; <label>:10:                                     ; preds = %3
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
define i32 @main() #0 !dbg !33 {
  %1 = alloca [50 x i8], align 16
  %2 = alloca [10 x i8], align 1
  call void @llvm.dbg.declare(metadata [50 x i8]* %1, metadata !36, metadata !DIExpression()), !dbg !40
  %3 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i32 0, i32 0, !dbg !41
  %4 = getelementptr inbounds [34 x i8], [34 x i8]* @.str, i32 0, i32 0, !dbg !42
  %5 = call i8* @strcpy(i8* %3, i8* %4), !dbg !42
  %6 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i32 0, i32 0, !dbg !43
  %7 = call i32 @puts(i8* %6), !dbg !44
  %8 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i32 0, i32 0, !dbg !45
  call void @llvm.memset.p0i8.i64(i8* align 16 %8, i8 36, i64 50, i1 false), !dbg !45
  %9 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i32 0, i32 0, !dbg !46
  %10 = call i8* @foo(i8* %9, i32 10), !dbg !47
  call void @llvm.dbg.value(metadata i8* %10, metadata !48, metadata !DIExpression()), !dbg !49
  call void @llvm.dbg.declare(metadata [10 x i8]* %2, metadata !50, metadata !DIExpression()), !dbg !54
  %11 = getelementptr inbounds [10 x i8], [10 x i8]* %2, i32 0, i32 0, !dbg !55
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %11, i8* align 1 %10, i64 10, i1 false), !dbg !55
  %12 = getelementptr inbounds [10 x i8], [10 x i8]* %2, i32 0, i32 0, !dbg !56
  %13 = call i8* @foo(i8* %12, i32 10), !dbg !57
  call void @llvm.dbg.value(metadata i8* %13, metadata !58, metadata !DIExpression()), !dbg !59
  %14 = call i32 @puts(i8* %13), !dbg !60
  ret i32 0, !dbg !61
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

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 (tags/RELEASE_800/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "local-array-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
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
!43 = !DILocation(line: 15, column: 8, scope: !33)
!44 = !DILocation(line: 15, column: 3, scope: !33)
!45 = !DILocation(line: 17, column: 3, scope: !33)
!46 = !DILocation(line: 18, column: 17, scope: !33)
!47 = !DILocation(line: 18, column: 13, scope: !33)
!48 = !DILocalVariable(name: "A", scope: !33, file: !1, line: 18, type: !11)
!49 = !DILocation(line: 18, column: 9, scope: !33)
!50 = !DILocalVariable(name: "B", scope: !33, file: !1, line: 20, type: !51)
!51 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 80, elements: !52)
!52 = !{!53}
!53 = !DISubrange(count: 10)
!54 = !DILocation(line: 20, column: 8, scope: !33)
!55 = !DILocation(line: 21, column: 3, scope: !33)
!56 = !DILocation(line: 22, column: 17, scope: !33)
!57 = !DILocation(line: 22, column: 13, scope: !33)
!58 = !DILocalVariable(name: "C", scope: !33, file: !1, line: 22, type: !11)
!59 = !DILocation(line: 22, column: 9, scope: !33)
!60 = !DILocation(line: 23, column: 3, scope: !33)
!61 = !DILocation(line: 24, column: 3, scope: !33)
