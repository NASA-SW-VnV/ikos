; ModuleID = 'local-array-2.c.pp.bc'
source_filename = "local-array-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

@.str = private unnamed_addr constant [34 x i8] c"This is string.h library function\00", align 1
; CHECK: define [34 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [84, 104, 105, 115, 32, 105, 115, 32, 115, 116, 114, 105, 110, 103, 46, 104, 32, 108, 105, 98, 114, 97, 114, 121, 32, 102, 117, 110, 99, 116, 105, 111, 110, 0], align 1
; CHECK: }
; CHECK: }

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #2
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #2
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc i8* @foo(i8*, i32) unnamed_addr #0 !dbg !7 {
  call void @llvm.dbg.value(metadata i8* %0, i64 0, metadata !13, metadata !14), !dbg !15
  call void @llvm.dbg.value(metadata i32 %1, i64 0, metadata !16, metadata !14), !dbg !17
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !18, metadata !14), !dbg !19
  br label %3, !dbg !20

; <label>:3:                                      ; preds = %5, %2
  %.0 = phi i32 [ 0, %2 ], [ %8, %5 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !18, metadata !14), !dbg !19
  %4 = icmp slt i32 %.0, %1, !dbg !22
  br i1 %4, label %5, label %9, !dbg !25

; <label>:5:                                      ; preds = %3
  %6 = sext i32 %.0 to i64, !dbg !27
  %7 = getelementptr inbounds i8, i8* %0, i64 %6, !dbg !27
  store i8 65, i8* %7, align 1, !dbg !28
  %8 = add nsw i32 %.0, 1, !dbg !29
  call void @llvm.dbg.value(metadata i32 %8, i64 0, metadata !18, metadata !14), !dbg !19
  br label %3, !dbg !31, !llvm.loop !32

; <label>:9:                                      ; preds = %3
  ret i8* %0, !dbg !35
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() local_unnamed_addr #0 !dbg !36 {
  %1 = alloca [50 x i8], align 16
  %2 = alloca [10 x i8], align 1
  call void @llvm.dbg.declare(metadata [50 x i8]* %1, metadata !39, metadata !14), !dbg !43
  %3 = getelementptr inbounds [50 x i8], [50 x i8]* %1, i64 0, i64 0, !dbg !44
  %4 = getelementptr inbounds [34 x i8], [34 x i8]* @.str, i64 0, i64 0, !dbg !45
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %3, i8* %4, i64 34, i32 1, i1 false), !dbg !45
  call void @llvm.memset.p0i8.i64(i8* %3, i8 36, i64 50, i32 16, i1 false), !dbg !46
  %5 = call fastcc i8* @foo(i8* %3, i32 10), !dbg !47
  call void @llvm.dbg.value(metadata i8* %5, i64 0, metadata !48, metadata !14), !dbg !49
  call void @llvm.dbg.declare(metadata [10 x i8]* %2, metadata !50, metadata !14), !dbg !54
  %6 = getelementptr inbounds [10 x i8], [10 x i8]* %2, i64 0, i64 0, !dbg !55
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %6, i8* %5, i64 10, i32 1, i1 false), !dbg !55
  %7 = call fastcc i8* @foo(i8* %6, i32 10), !dbg !56
  call void @llvm.dbg.value(metadata i8* %7, i64 0, metadata !57, metadata !14), !dbg !58
  ret i32 0, !dbg !59
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   [50 x si8]* $1 = allocate [50 x si8], 1, align 16
; CHECK:   [10 x si8]* $2 = allocate [10 x si8], 1, align 1
; CHECK:   si8* %3 = ptrshift $1, 50 * 0, 1 * 0
; CHECK:   si8* %4 = ptrshift @.str, 34 * 0, 1 * 0
; CHECK:   call @ar.memcpy(%3, %4, 34, 1, 0)
; CHECK:   call @ar.memset(%3, 36, 50, 16, 0)
; CHECK:   si8* %5 = call @foo(%3, 10)
; CHECK:   si8* %6 = ptrshift $2, 10 * 0, 1 * 0
; CHECK:   call @ar.memcpy(%6, %5, 10, 1, 0)
; CHECK:   si8* %7 = call @foo(%6, 10)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "local-array-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 4, type: !8, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !12}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 4, type: !10)
!14 = !DIExpression()
!15 = !DILocation(line: 4, column: 17, scope: !7)
!16 = !DILocalVariable(name: "n", arg: 2, scope: !7, file: !1, line: 4, type: !12)
!17 = !DILocation(line: 4, column: 24, scope: !7)
!18 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 5, type: !12)
!19 = !DILocation(line: 5, column: 7, scope: !7)
!20 = !DILocation(line: 6, column: 8, scope: !21)
!21 = distinct !DILexicalBlock(scope: !7, file: !1, line: 6, column: 3)
!22 = !DILocation(line: 6, column: 17, scope: !23)
!23 = !DILexicalBlockFile(scope: !24, file: !1, discriminator: 1)
!24 = distinct !DILexicalBlock(scope: !21, file: !1, line: 6, column: 3)
!25 = !DILocation(line: 6, column: 3, scope: !26)
!26 = !DILexicalBlockFile(scope: !21, file: !1, discriminator: 1)
!27 = !DILocation(line: 7, column: 5, scope: !24)
!28 = !DILocation(line: 7, column: 10, scope: !24)
!29 = !DILocation(line: 6, column: 23, scope: !30)
!30 = !DILexicalBlockFile(scope: !24, file: !1, discriminator: 2)
!31 = !DILocation(line: 6, column: 3, scope: !30)
!32 = distinct !{!32, !33, !34}
!33 = !DILocation(line: 6, column: 3, scope: !21)
!34 = !DILocation(line: 7, column: 12, scope: !21)
!35 = !DILocation(line: 8, column: 3, scope: !7)
!36 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !37, isLocal: false, isDefinition: true, scopeLine: 11, isOptimized: false, unit: !0, variables: !2)
!37 = !DISubroutineType(types: !38)
!38 = !{!12}
!39 = !DILocalVariable(name: "str", scope: !36, file: !1, line: 12, type: !40)
!40 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 400, elements: !41)
!41 = !{!42}
!42 = !DISubrange(count: 50)
!43 = !DILocation(line: 12, column: 8, scope: !36)
!44 = !DILocation(line: 14, column: 10, scope: !36)
!45 = !DILocation(line: 14, column: 3, scope: !36)
!46 = !DILocation(line: 17, column: 3, scope: !36)
!47 = !DILocation(line: 18, column: 13, scope: !36)
!48 = !DILocalVariable(name: "A", scope: !36, file: !1, line: 18, type: !10)
!49 = !DILocation(line: 18, column: 9, scope: !36)
!50 = !DILocalVariable(name: "B", scope: !36, file: !1, line: 20, type: !51)
!51 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 80, elements: !52)
!52 = !{!53}
!53 = !DISubrange(count: 10)
!54 = !DILocation(line: 20, column: 8, scope: !36)
!55 = !DILocation(line: 21, column: 3, scope: !36)
!56 = !DILocation(line: 22, column: 13, scope: !36)
!57 = !DILocalVariable(name: "C", scope: !36, file: !1, line: 22, type: !10)
!58 = !DILocation(line: 22, column: 9, scope: !36)
!59 = !DILocation(line: 24, column: 3, scope: !36)
