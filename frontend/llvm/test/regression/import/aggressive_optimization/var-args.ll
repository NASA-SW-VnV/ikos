; ModuleID = 'var-args.pp.bc'
source_filename = "var-args.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.__va_list_tag = type { i32, i32, i8*, i8* }

@.str = private unnamed_addr constant [6 x i8] c"[%d] \00", align 1
; CHECK: define [6 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [91, 37, 100, 93, 32, 0], align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal void @PrintInts(i32, ...) unnamed_addr #0 !dbg !11 {
  %2 = alloca [1 x %struct.__va_list_tag], align 16
  %3 = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata !2, metadata !17, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i32 0, metadata !20, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i32 %0, metadata !21, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.declare(metadata [1 x %struct.__va_list_tag]* %2, metadata !22, metadata !DIExpression()), !dbg !39
  call void @llvm.dbg.declare(metadata [1 x %struct.__va_list_tag]* %3, metadata !40, metadata !DIExpression()), !dbg !41
  %4 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %2, i64 0, i64 0, !dbg !42
  %5 = bitcast [1 x %struct.__va_list_tag]* %2 to i8*, !dbg !42
  call void @llvm.va_start(i8* %5), !dbg !42
  %6 = bitcast [1 x %struct.__va_list_tag]* %3 to i8*, !dbg !43
  call void @llvm.va_copy(i8* %6, i8* %5), !dbg !43
  %7 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %3, i64 0, i64 0, i32 0, !dbg !44
  %8 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %3, i64 0, i64 0, i32 2, !dbg !44
  %9 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %3, i64 0, i64 0, i32 3, !dbg !44
  br label %10, !dbg !46

10:                                               ; preds = %23, %1
  %.01 = phi i32 [ 0, %1 ], [ %26, %23 ], !dbg !16
  %.0 = phi i32 [ %0, %1 ], [ %25, %23 ], !dbg !16
  call void @llvm.dbg.value(metadata i32 %.0, metadata !21, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i32 %.01, metadata !20, metadata !DIExpression()), !dbg !16
  %11 = icmp eq i32 %.0, 0, !dbg !47
  br i1 %11, label %27, label %12, !dbg !46

12:                                               ; preds = %10
  %13 = load i32, i32* %7, align 16, !dbg !48
  %14 = icmp ult i32 %13, 41, !dbg !48
  br i1 %14, label %15, label %20, !dbg !48

15:                                               ; preds = %12
  %16 = load i8*, i8** %9, align 16, !dbg !48
  %17 = sext i32 %13 to i64, !dbg !48
  %18 = getelementptr i8, i8* %16, i64 %17, !dbg !48
  %19 = add i32 %13, 8, !dbg !48
  store i32 %19, i32* %7, align 16, !dbg !48
  br label %23, !dbg !48

20:                                               ; preds = %12
  %21 = load i8*, i8** %8, align 8, !dbg !48
  %22 = getelementptr i8, i8* %21, i64 8, !dbg !48
  store i8* %22, i8** %8, align 8, !dbg !48
  br label %23, !dbg !48

23:                                               ; preds = %20, %15
  %.in = phi i8* [ %18, %15 ], [ %21, %20 ]
  %24 = bitcast i8* %.in to i32*, !dbg !48
  %25 = load i32, i32* %24, align 4, !dbg !48
  call void @llvm.dbg.value(metadata i32 %25, metadata !21, metadata !DIExpression()), !dbg !16
  %26 = add nuw nsw i32 %.01, 1, !dbg !49
  call void @llvm.dbg.value(metadata i32 %26, metadata !20, metadata !DIExpression()), !dbg !16
  br label %10, !dbg !46, !llvm.loop !50

27:                                               ; preds = %10
  %.01.lcssa = phi i32 [ %.01, %10 ], !dbg !16
  call void @llvm.dbg.value(metadata i32 %.01.lcssa, metadata !20, metadata !DIExpression()), !dbg !16
  call void @llvm.va_end(i8* nonnull %6), !dbg !52
  %28 = zext i32 %.01.lcssa to i64, !dbg !53
  %29 = mul nuw nsw i64 %28, 5, !dbg !54
  %30 = add nuw nsw i64 %29, 1, !dbg !55
  %31 = call i8* @malloc(i64 %30) #7, !dbg !56
  call void @llvm.dbg.value(metadata i8* %31, metadata !57, metadata !DIExpression()), !dbg !16
  store i8 0, i8* %31, align 1, !dbg !58
  br label %32, !dbg !59

32:                                               ; preds = %34, %27
  %.1 = phi i32 [ %.01.lcssa, %27 ], [ %36, %34 ], !dbg !16
  call void @llvm.dbg.value(metadata i32 %.1, metadata !20, metadata !DIExpression()), !dbg !16
  %33 = icmp sgt i32 %.1, 0, !dbg !60
  br i1 %33, label %34, label %37, !dbg !63

34:                                               ; preds = %32
  %strlen = call i64 @strlen(i8* %31), !dbg !64
  %endptr = getelementptr i8, i8* %31, i64 %strlen, !dbg !64
  %35 = getelementptr inbounds [6 x i8], [6 x i8]* @.str, i64 0, i64 0, !dbg !64
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %endptr, i8* align 1 %35, i64 6, i1 false), !dbg !64
  %36 = add nsw i32 %.1, -1, !dbg !66
  call void @llvm.dbg.value(metadata i32 %36, metadata !20, metadata !DIExpression()), !dbg !16
  br label %32, !dbg !67, !llvm.loop !68

37:                                               ; preds = %32
  %38 = call i32 @vprintf(i8* %31, %struct.__va_list_tag* nonnull %4) #2, !dbg !70
  call void @llvm.va_end(i8* nonnull %5), !dbg !71
  ret void, !dbg !72
}
; CHECK: define void @PrintInts(si32 %1, ...) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}]* $2 = allocate [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}], 1, align 16
; CHECK:   [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}]* $3 = allocate [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}], 1, align 16
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %4 = ptrshift $2, 24 * 0, 24 * 0
; CHECK:   si8* %5 = bitcast $2
; CHECK:   call @ar.va_start(%5)
; CHECK:   si8* %6 = bitcast $3
; CHECK:   call @ar.va_copy(%6, %5)
; CHECK:   si32* %7 = ptrshift $3, 24 * 0, 24 * 0, 1 * 0
; CHECK:   si8** %8 = ptrshift $3, 24 * 0, 24 * 0, 1 * 8
; CHECK:   si8** %9 = ptrshift $3, 24 * 0, 24 * 0, 1 * 16
; CHECK:   si32 %.01 = 0
; CHECK:   si32 %.0 = %1
; CHECK: }
; CHECK: #2 predecessors={#1, #10} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#5} {
; CHECK:   %.0 sieq 0
; CHECK:   si32 %.01.lcssa = %.01
; CHECK:   call @ar.va_end(%6)
; CHECK:   ui32 %10 = bitcast %.01.lcssa
; CHECK:   ui64 %11 = zext %10
; CHECK:   si64 %12 = bitcast %11
; CHECK:   si64 %13 = %12 smul.nw 5
; CHECK:   si64 %14 = %13 sadd.nw 1
; CHECK:   ui64 %15 = bitcast %14
; CHECK:   si8* %16 = call @ar.libc.malloc(%15)
; CHECK:   store %16, 0, align 1
; CHECK:   si32 %.1 = %.01.lcssa
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#6, #7} {
; CHECK:   %.0 sine 0
; CHECK:   ui32* %17 = bitcast %7
; CHECK:   ui32 %18 = load %17, align 16
; CHECK: }
; CHECK: #6 predecessors={#4} successors={#10} {
; CHECK:   %18 uilt 41
; CHECK:   si8* %19 = load %9, align 16
; CHECK:   si32 %20 = bitcast %18
; CHECK:   si64 %21 = sext %20
; CHECK:   si8* %22 = ptrshift %19, 1 * %21
; CHECK:   ui32 %23 = %18 uadd 8
; CHECK:   si32 %24 = bitcast %23
; CHECK:   store %7, %24, align 16
; CHECK:   si8* %.in = %22
; CHECK: }
; CHECK: #7 predecessors={#4} successors={#10} {
; CHECK:   %18 uige 41
; CHECK:   si8* %25 = load %8, align 8
; CHECK:   si8* %26 = ptrshift %25, 1 * 8
; CHECK:   store %8, %26, align 8
; CHECK:   si8* %.in = %25
; CHECK: }
; CHECK: #5 predecessors={#3, #8} successors={#8, #9} {
; CHECK: }
; CHECK: #8 predecessors={#5} successors={#5} {
; CHECK:   %.1 sigt 0
; CHECK:   ui64 %strlen = call @ar.libc.strlen(%16)
; CHECK:   si8* %endptr = ptrshift %16, 1 * %strlen
; CHECK:   si8* %27 = ptrshift @.str, 6 * 0, 1 * 0
; CHECK:   call @ar.memcpy(%endptr, %27, 6, 1, 1, 0)
; CHECK:   si32 %28 = %.1 sadd.nw -1
; CHECK:   si32 %.1 = %28
; CHECK: }
; CHECK: #9 !exit predecessors={#5} {
; CHECK:   %.1 sile 0
; CHECK:   si32 %29 = call @vprintf(%16, %4)
; CHECK:   call @ar.va_end(%5)
; CHECK:   return
; CHECK: }
; CHECK: #10 predecessors={#6, #7} successors={#2} {
; CHECK:   si32* %30 = bitcast %.in
; CHECK:   si32 %31 = load %30, align 4
; CHECK:   si32 %32 = %.01 sadd.nw 1
; CHECK:   si32 %.01 = %32
; CHECK:   si32 %.0 = %31
; CHECK: }
; CHECK: }

; Function Attrs: allocsize(0)
declare i8* @malloc(i64) local_unnamed_addr #3
; CHECK: declare si8* @ar.libc.malloc(ui64)

; Function Attrs: argmemonly nofree nounwind readonly
declare i64 @strlen(i8* nocapture) local_unnamed_addr #4
; CHECK: declare ui64 @ar.libc.strlen(si8*)

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #6
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: nounwind
declare void @llvm.va_copy(i8*, i8*) #2
; CHECK: declare void @ar.va_copy(si8*, si8*)

; Function Attrs: nounwind
declare void @llvm.va_end(i8*) #2
; CHECK: declare void @ar.va_end(si8*)

; Function Attrs: nounwind
declare void @llvm.va_start(i8*) #2
; CHECK: declare void @ar.va_start(si8*)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() local_unnamed_addr #0 !dbg !73 {
  call void (i32, ...) @PrintInts(i32 10, i32 20, i32 30, i32 40, i32 50, i32 0), !dbg !76
  ret i32 0, !dbg !77
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   call @PrintInts(10, 20, 30, 40, 50, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

declare i32 @vprintf(i8*, %struct.__va_list_tag*) local_unnamed_addr #5
; CHECK: declare si32 @vprintf(si8*, {0: si32, 4: si32, 8: si8*, 16: si8*}*)

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }
attributes #3 = { allocsize(0) "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { argmemonly nofree nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #6 = { argmemonly nounwind }
attributes #7 = { nounwind allocsize(0) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!6, !7, !8, !9}
!llvm.ident = !{!10}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "var-args.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4}
!4 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!5 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{i32 7, !"PIC Level", i32 2}
!10 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!11 = distinct !DISubprogram(name: "PrintInts", scope: !1, file: !1, line: 8, type: !12, scopeLine: 8, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!12 = !DISubroutineType(types: !13)
!13 = !{null, !14, null}
!14 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!15 = !DILocalVariable(name: "first", arg: 1, scope: !11, file: !1, line: 8, type: !14)
!16 = !DILocation(line: 0, scope: !11)
!17 = !DILocalVariable(name: "format", scope: !11, file: !1, line: 10, type: !18)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!19 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !5)
!20 = !DILocalVariable(name: "count", scope: !11, file: !1, line: 11, type: !14)
!21 = !DILocalVariable(name: "val", scope: !11, file: !1, line: 12, type: !14)
!22 = !DILocalVariable(name: "vl", scope: !11, file: !1, line: 13, type: !23)
!23 = !DIDerivedType(tag: DW_TAG_typedef, name: "va_list", file: !24, line: 32, baseType: !25)
!24 = !DIFile(filename: "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/sys/_types/_va_list.h", directory: "")
!25 = !DIDerivedType(tag: DW_TAG_typedef, name: "__darwin_va_list", file: !26, line: 98, baseType: !27)
!26 = !DIFile(filename: "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/i386/_types.h", directory: "")
!27 = !DIDerivedType(tag: DW_TAG_typedef, name: "__builtin_va_list", file: !1, line: 13, baseType: !28)
!28 = !DICompositeType(tag: DW_TAG_array_type, baseType: !29, size: 192, elements: !37)
!29 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__va_list_tag", file: !1, line: 13, size: 192, elements: !30)
!30 = !{!31, !33, !34, !36}
!31 = !DIDerivedType(tag: DW_TAG_member, name: "gp_offset", scope: !29, file: !1, line: 13, baseType: !32, size: 32)
!32 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "fp_offset", scope: !29, file: !1, line: 13, baseType: !32, size: 32, offset: 32)
!34 = !DIDerivedType(tag: DW_TAG_member, name: "overflow_arg_area", scope: !29, file: !1, line: 13, baseType: !35, size: 64, offset: 64)
!35 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!36 = !DIDerivedType(tag: DW_TAG_member, name: "reg_save_area", scope: !29, file: !1, line: 13, baseType: !35, size: 64, offset: 128)
!37 = !{!38}
!38 = !DISubrange(count: 1)
!39 = !DILocation(line: 13, column: 11, scope: !11)
!40 = !DILocalVariable(name: "vl_count", scope: !11, file: !1, line: 13, type: !23)
!41 = !DILocation(line: 13, column: 15, scope: !11)
!42 = !DILocation(line: 14, column: 3, scope: !11)
!43 = !DILocation(line: 17, column: 3, scope: !11)
!44 = !DILocation(line: 0, scope: !45)
!45 = distinct !DILexicalBlock(scope: !11, file: !1, line: 18, column: 20)
!46 = !DILocation(line: 18, column: 3, scope: !11)
!47 = !DILocation(line: 18, column: 14, scope: !11)
!48 = !DILocation(line: 19, column: 11, scope: !45)
!49 = !DILocation(line: 20, column: 5, scope: !45)
!50 = distinct !{!50, !46, !51}
!51 = !DILocation(line: 21, column: 3, scope: !11)
!52 = !DILocation(line: 22, column: 3, scope: !11)
!53 = !DILocation(line: 25, column: 43, scope: !11)
!54 = !DILocation(line: 25, column: 41, scope: !11)
!55 = !DILocation(line: 25, column: 49, scope: !11)
!56 = !DILocation(line: 25, column: 19, scope: !11)
!57 = !DILocalVariable(name: "buffer", scope: !11, file: !1, line: 9, type: !4)
!58 = !DILocation(line: 26, column: 13, scope: !11)
!59 = !DILocation(line: 29, column: 3, scope: !11)
!60 = !DILocation(line: 29, column: 16, scope: !61)
!61 = distinct !DILexicalBlock(scope: !62, file: !1, line: 29, column: 3)
!62 = distinct !DILexicalBlock(scope: !11, file: !1, line: 29, column: 3)
!63 = !DILocation(line: 29, column: 3, scope: !62)
!64 = !DILocation(line: 30, column: 5, scope: !65)
!65 = distinct !DILexicalBlock(scope: !61, file: !1, line: 29, column: 30)
!66 = !DILocation(line: 29, column: 21, scope: !61)
!67 = !DILocation(line: 29, column: 3, scope: !61)
!68 = distinct !{!68, !63, !69}
!69 = !DILocation(line: 31, column: 3, scope: !62)
!70 = !DILocation(line: 35, column: 3, scope: !11)
!71 = !DILocation(line: 37, column: 3, scope: !11)
!72 = !DILocation(line: 38, column: 1, scope: !11)
!73 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 40, type: !74, scopeLine: 40, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!74 = !DISubroutineType(types: !75)
!75 = !{!14}
!76 = !DILocation(line: 41, column: 3, scope: !73)
!77 = !DILocation(line: 42, column: 3, scope: !73)
