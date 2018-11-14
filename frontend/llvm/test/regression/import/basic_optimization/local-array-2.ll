; ModuleID = 'local-array-2.c.pp.bc'
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
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #3
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #3
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i8* @foo(i8*, i32) #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i8* %0, metadata !14, metadata !DIExpression()), !dbg !15
  call void @llvm.dbg.value(metadata i32 %1, metadata !16, metadata !DIExpression()), !dbg !17
  call void @llvm.dbg.value(metadata i32 0, metadata !18, metadata !DIExpression()), !dbg !19
  br label %3, !dbg !20

; <label>:3:                                      ; preds = %8, %2
  %.0 = phi i32 [ 0, %2 ], [ %9, %8 ]
  call void @llvm.dbg.value(metadata i32 %.0, metadata !18, metadata !DIExpression()), !dbg !19
  %4 = icmp slt i32 %.0, %1, !dbg !22
  br i1 %4, label %5, label %10, !dbg !24

; <label>:5:                                      ; preds = %3
  %6 = sext i32 %.0 to i64, !dbg !25
  %7 = getelementptr inbounds i8, i8* %0, i64 %6, !dbg !25
  store i8 65, i8* %7, align 1, !dbg !26
  br label %8, !dbg !25

; <label>:8:                                      ; preds = %5
  %9 = add nsw i32 %.0, 1, !dbg !27
  call void @llvm.dbg.value(metadata i32 %9, metadata !18, metadata !DIExpression()), !dbg !19
  br label %3, !dbg !28, !llvm.loop !29

; <label>:10:                                     ; preds = %3
  ret i8* %0, !dbg !31
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

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !32 {
  %1 = alloca [50 x i8], align 16
  %2 = alloca [10 x i8], align 1
  call void @llvm.dbg.declare(metadata [50 x i8]* %1, metadata !35, metadata !DIExpression()), !dbg !39
  %3 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i32 0, i32 0, !dbg !40
  %4 = getelementptr inbounds [34 x i8], [34 x i8]* @.str, i32 0, i32 0, !dbg !41
  %5 = call i8* @strcpy(i8* %3, i8* %4), !dbg !41
  %6 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i32 0, i32 0, !dbg !42
  %7 = call i32 @puts(i8* %6), !dbg !43
  %8 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i32 0, i32 0, !dbg !44
  call void @llvm.memset.p0i8.i64(i8* %8, i8 36, i64 50, i32 16, i1 false), !dbg !44
  %9 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i32 0, i32 0, !dbg !45
  %10 = call i8* @foo(i8* %9, i32 10), !dbg !46
  call void @llvm.dbg.value(metadata i8* %10, metadata !47, metadata !DIExpression()), !dbg !48
  call void @llvm.dbg.declare(metadata [10 x i8]* %2, metadata !49, metadata !DIExpression()), !dbg !53
  %11 = getelementptr inbounds [10 x i8], [10 x i8]* %2, i32 0, i32 0, !dbg !54
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %11, i8* %10, i64 10, i32 1, i1 false), !dbg !54
  %12 = getelementptr inbounds [10 x i8], [10 x i8]* %2, i32 0, i32 0, !dbg !55
  %13 = call i8* @foo(i8* %12, i32 10), !dbg !56
  call void @llvm.dbg.value(metadata i8* %13, metadata !57, metadata !DIExpression()), !dbg !58
  %14 = call i32 @puts(i8* %13), !dbg !59
  ret i32 0, !dbg !60
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
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 6.0.1 (tags/RELEASE_601/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "local-array-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 6.0.1 (tags/RELEASE_601/final)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 4, type: !9, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
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
!22 = !DILocation(line: 6, column: 17, scope: !23)
!23 = distinct !DILexicalBlock(scope: !21, file: !1, line: 6, column: 3)
!24 = !DILocation(line: 6, column: 3, scope: !21)
!25 = !DILocation(line: 7, column: 5, scope: !23)
!26 = !DILocation(line: 7, column: 10, scope: !23)
!27 = !DILocation(line: 6, column: 23, scope: !23)
!28 = !DILocation(line: 6, column: 3, scope: !23)
!29 = distinct !{!29, !24, !30}
!30 = !DILocation(line: 7, column: 12, scope: !21)
!31 = !DILocation(line: 8, column: 3, scope: !8)
!32 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !33, isLocal: false, isDefinition: true, scopeLine: 11, isOptimized: false, unit: !0, variables: !2)
!33 = !DISubroutineType(types: !34)
!34 = !{!13}
!35 = !DILocalVariable(name: "str", scope: !32, file: !1, line: 12, type: !36)
!36 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 400, elements: !37)
!37 = !{!38}
!38 = !DISubrange(count: 50)
!39 = !DILocation(line: 12, column: 8, scope: !32)
!40 = !DILocation(line: 14, column: 10, scope: !32)
!41 = !DILocation(line: 14, column: 3, scope: !32)
!42 = !DILocation(line: 15, column: 8, scope: !32)
!43 = !DILocation(line: 15, column: 3, scope: !32)
!44 = !DILocation(line: 17, column: 3, scope: !32)
!45 = !DILocation(line: 18, column: 17, scope: !32)
!46 = !DILocation(line: 18, column: 13, scope: !32)
!47 = !DILocalVariable(name: "A", scope: !32, file: !1, line: 18, type: !11)
!48 = !DILocation(line: 18, column: 9, scope: !32)
!49 = !DILocalVariable(name: "B", scope: !32, file: !1, line: 20, type: !50)
!50 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 80, elements: !51)
!51 = !{!52}
!52 = !DISubrange(count: 10)
!53 = !DILocation(line: 20, column: 8, scope: !32)
!54 = !DILocation(line: 21, column: 3, scope: !32)
!55 = !DILocation(line: 22, column: 17, scope: !32)
!56 = !DILocation(line: 22, column: 13, scope: !32)
!57 = !DILocalVariable(name: "C", scope: !32, file: !1, line: 22, type: !11)
!58 = !DILocation(line: 22, column: 9, scope: !32)
!59 = !DILocation(line: 23, column: 3, scope: !32)
!60 = !DILocation(line: 24, column: 3, scope: !32)
