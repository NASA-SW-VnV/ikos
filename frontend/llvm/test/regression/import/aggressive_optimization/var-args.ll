; ModuleID = 'var-args.c.pp.bc'
source_filename = "var-args.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

%struct.__va_list_tag = type { i32, i32, i8*, i8* }

@.str = private unnamed_addr constant [6 x i8] c"[%d] \00", align 1
; CHECK: define [6 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [91, 37, 100, 93, 32, 0], align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal void @PrintInts(i32, ...) unnamed_addr #0 !dbg !10 {
  %2 = alloca [1 x %struct.__va_list_tag], align 16
  %3 = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !14, metadata !15), !dbg !16
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !17, metadata !15), !dbg !18
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !19, metadata !15), !dbg !20
  call void @llvm.dbg.declare(metadata [1 x %struct.__va_list_tag]* %2, metadata !21, metadata !15), !dbg !38
  call void @llvm.dbg.declare(metadata [1 x %struct.__va_list_tag]* %3, metadata !39, metadata !15), !dbg !40
  %4 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %2, i64 0, i64 0, !dbg !41
  %5 = bitcast [1 x %struct.__va_list_tag]* %2 to i8*, !dbg !41
  call void @llvm.va_start(i8* %5), !dbg !41
  %6 = bitcast [1 x %struct.__va_list_tag]* %3 to i8*, !dbg !42
  call void @llvm.va_copy(i8* %6, i8* %5), !dbg !42
  br label %7, !dbg !43

; <label>:7:                                      ; preds = %23, %1
  %.01 = phi i32 [ 0, %1 ], [ %26, %23 ]
  %.0 = phi i32 [ %0, %1 ], [ %25, %23 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !19, metadata !15), !dbg !20
  call void @llvm.dbg.value(metadata i32 %.01, i64 0, metadata !17, metadata !15), !dbg !18
  %8 = icmp eq i32 %.0, 0, !dbg !44
  br i1 %8, label %27, label %9, !dbg !46

; <label>:9:                                      ; preds = %7
  %10 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %3, i64 0, i64 0, i32 0, !dbg !47
  %11 = load i32, i32* %10, align 16, !dbg !47
  %12 = icmp ult i32 %11, 41, !dbg !47
  br i1 %12, label %13, label %19, !dbg !47

; <label>:13:                                     ; preds = %9
  %14 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %3, i64 0, i64 0, i32 3, !dbg !49
  %15 = load i8*, i8** %14, align 16, !dbg !49
  %16 = sext i32 %11 to i64, !dbg !49
  %17 = getelementptr i8, i8* %15, i64 %16, !dbg !49
  %18 = add i32 %11, 8, !dbg !49
  store i32 %18, i32* %10, align 16, !dbg !49
  br label %23, !dbg !49

; <label>:19:                                     ; preds = %9
  %20 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %3, i64 0, i64 0, i32 2, !dbg !51
  %21 = load i8*, i8** %20, align 8, !dbg !51
  %22 = getelementptr i8, i8* %21, i64 8, !dbg !51
  store i8* %22, i8** %20, align 8, !dbg !51
  br label %23, !dbg !51

; <label>:23:                                     ; preds = %19, %13
  %.in = phi i8* [ %17, %13 ], [ %21, %19 ]
  %24 = bitcast i8* %.in to i32*, !dbg !53
  %25 = load i32, i32* %24, align 4, !dbg !53
  call void @llvm.dbg.value(metadata i32 %25, i64 0, metadata !19, metadata !15), !dbg !20
  %26 = add nsw i32 %.01, 1, !dbg !55
  call void @llvm.dbg.value(metadata i32 %26, i64 0, metadata !17, metadata !15), !dbg !18
  br label %7, !dbg !56, !llvm.loop !58

; <label>:27:                                     ; preds = %7
  %.01.lcssa = phi i32 [ %.01, %7 ]
  call void @llvm.va_end(i8* %6), !dbg !60
  %28 = sext i32 %.01.lcssa to i64, !dbg !61
  %29 = mul nsw i64 %28, 5, !dbg !62
  %30 = add i64 %29, 1, !dbg !63
  %31 = call i8* @malloc(i64 %30) #2, !dbg !64
  call void @llvm.dbg.value(metadata i8* %31, i64 0, metadata !65, metadata !15), !dbg !66
  store i8 0, i8* %31, align 1, !dbg !67
  br label %32, !dbg !68

; <label>:32:                                     ; preds = %34, %27
  %.1 = phi i32 [ %.01.lcssa, %27 ], [ %36, %34 ]
  call void @llvm.dbg.value(metadata i32 %.1, i64 0, metadata !17, metadata !15), !dbg !18
  %33 = icmp sgt i32 %.1, 0, !dbg !69
  br i1 %33, label %34, label %37, !dbg !73

; <label>:34:                                     ; preds = %32
  %strlen = call i64 @strlen(i8* %31), !dbg !75
  %endptr = getelementptr i8, i8* %31, i64 %strlen, !dbg !75
  %35 = getelementptr inbounds [6 x i8], [6 x i8]* @.str, i64 0, i64 0, !dbg !75
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %endptr, i8* %35, i64 6, i32 1, i1 false), !dbg !75
  %36 = add nsw i32 %.1, -1, !dbg !77
  call void @llvm.dbg.value(metadata i32 %36, i64 0, metadata !17, metadata !15), !dbg !18
  br label %32, !dbg !79, !llvm.loop !80

; <label>:37:                                     ; preds = %32
  %38 = call i32 @vprintf(i8* %31, %struct.__va_list_tag* %4) #2, !dbg !83
  call void @llvm.va_end(i8* %5), !dbg !84
  ret void, !dbg !85
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
; CHECK:   si32 %.01 = 0
; CHECK:   si32 %.0 = %1
; CHECK: }
; CHECK: #2 predecessors={#1, #10} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#5} {
; CHECK:   %.0 sieq 0
; CHECK:   si32 %.01.lcssa = %.01
; CHECK:   call @ar.va_end(%6)
; CHECK:   si64 %7 = sext %.01.lcssa
; CHECK:   si64 %8 = %7 smul.nw 5
; CHECK:   ui64 %9 = bitcast %8
; CHECK:   ui64 %10 = %9 uadd 1
; CHECK:   si8* %11 = call @ar.libc.malloc(%10)
; CHECK:   store %11, 0, align 1
; CHECK:   si32 %.1 = %.01.lcssa
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#6, #7} {
; CHECK:   %.0 sine 0
; CHECK:   si32* %12 = ptrshift $3, 24 * 0, 24 * 0, 1 * 0
; CHECK:   ui32* %13 = bitcast %12
; CHECK:   ui32 %14 = load %13, align 16
; CHECK: }
; CHECK: #6 predecessors={#4} successors={#10} {
; CHECK:   %14 uilt 41
; CHECK:   si8** %15 = ptrshift $3, 24 * 0, 24 * 0, 1 * 16
; CHECK:   si8* %16 = load %15, align 16
; CHECK:   si32 %17 = bitcast %14
; CHECK:   si64 %18 = sext %17
; CHECK:   si8* %19 = ptrshift %16, 1 * %18
; CHECK:   ui32 %20 = %14 uadd 8
; CHECK:   si32 %21 = bitcast %20
; CHECK:   store %12, %21, align 16
; CHECK:   si8* %.in = %19
; CHECK: }
; CHECK: #7 predecessors={#4} successors={#10} {
; CHECK:   %14 uige 41
; CHECK:   si8** %22 = ptrshift $3, 24 * 0, 24 * 0, 1 * 8
; CHECK:   si8* %23 = load %22, align 8
; CHECK:   si8* %24 = ptrshift %23, 1 * 8
; CHECK:   store %22, %24, align 8
; CHECK:   si8* %.in = %23
; CHECK: }
; CHECK: #5 predecessors={#3, #8} successors={#8, #9} {
; CHECK: }
; CHECK: #8 predecessors={#5} successors={#5} {
; CHECK:   %.1 sigt 0
; CHECK:   ui64 %strlen = call @ar.libc.strlen(%11)
; CHECK:   si8* %endptr = ptrshift %11, 1 * %strlen
; CHECK:   si8* %25 = ptrshift @.str, 6 * 0, 1 * 0
; CHECK:   call @ar.memcpy(%endptr, %25, 6, 1, 0)
; CHECK:   si32 %26 = %.1 sadd.nw -1
; CHECK:   si32 %.1 = %26
; CHECK: }
; CHECK: #9 !exit predecessors={#5} {
; CHECK:   %.1 sile 0
; CHECK:   si32 %27 = call @vprintf(%11, %4)
; CHECK:   call @ar.va_end(%5)
; CHECK:   return
; CHECK: }
; CHECK: #10 predecessors={#6, #7} successors={#2} {
; CHECK:   si32* %28 = bitcast %.in
; CHECK:   si32 %29 = load %28, align 4
; CHECK:   si32 %30 = %.01 sadd.nw 1
; CHECK:   si32 %.01 = %30
; CHECK:   si32 %.0 = %29
; CHECK: }
; CHECK: }

declare i8* @malloc(i64) local_unnamed_addr #3
; CHECK: declare si8* @ar.libc.malloc(ui64)

; Function Attrs: nounwind readonly
declare i64 @strlen(i8* nocapture) local_unnamed_addr #4
; CHECK: declare ui64 @ar.libc.strlen(si8*)

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #5
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui1)

; Function Attrs: nounwind
declare void @llvm.va_copy(i8*, i8*) #2
; CHECK: declare void @ar.va_copy(si8*, si8*)

; Function Attrs: nounwind
declare void @llvm.va_end(i8*) #2
; CHECK: declare void @ar.va_end(si8*)

; Function Attrs: nounwind
declare void @llvm.va_start(i8*) #2
; CHECK: declare void @ar.va_start(si8*)

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() local_unnamed_addr #0 !dbg !86 {
  call void (i32, ...) @PrintInts(i32 10, i32 20, i32 30, i32 40, i32 50, i32 0), !dbg !89
  ret i32 0, !dbg !90
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   call @PrintInts(10, 20, 30, 40, 50, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

declare i32 @vprintf(i8*, %struct.__va_list_tag*) local_unnamed_addr #3
; CHECK: declare si32 @vprintf(si8*, {0: si32, 4: si32, 8: si8*, 16: si8*}*)

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readonly "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3)
!1 = !DIFile(filename: "var-args.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4}
!4 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!5 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"PIC Level", i32 2}
!9 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!10 = distinct !DISubprogram(name: "PrintInts", scope: !1, file: !1, line: 8, type: !11, isLocal: false, isDefinition: true, scopeLine: 8, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!11 = !DISubroutineType(types: !12)
!12 = !{null, !13, null}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !DILocalVariable(name: "first", arg: 1, scope: !10, file: !1, line: 8, type: !13)
!15 = !DIExpression()
!16 = !DILocation(line: 8, column: 20, scope: !10)
!17 = !DILocalVariable(name: "count", scope: !10, file: !1, line: 11, type: !13)
!18 = !DILocation(line: 11, column: 7, scope: !10)
!19 = !DILocalVariable(name: "val", scope: !10, file: !1, line: 12, type: !13)
!20 = !DILocation(line: 12, column: 7, scope: !10)
!21 = !DILocalVariable(name: "vl", scope: !10, file: !1, line: 13, type: !22)
!22 = !DIDerivedType(tag: DW_TAG_typedef, name: "va_list", file: !23, line: 31, baseType: !24)
!23 = !DIFile(filename: "/usr/include/sys/_types/_va_list.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!24 = !DIDerivedType(tag: DW_TAG_typedef, name: "__darwin_va_list", file: !25, line: 98, baseType: !26)
!25 = !DIFile(filename: "/usr/include/i386/_types.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!26 = !DIDerivedType(tag: DW_TAG_typedef, name: "__builtin_va_list", file: !1, line: 13, baseType: !27)
!27 = !DICompositeType(tag: DW_TAG_array_type, baseType: !28, size: 192, elements: !36)
!28 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__va_list_tag", file: !1, line: 13, size: 192, elements: !29)
!29 = !{!30, !32, !33, !35}
!30 = !DIDerivedType(tag: DW_TAG_member, name: "gp_offset", scope: !28, file: !1, line: 13, baseType: !31, size: 32)
!31 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!32 = !DIDerivedType(tag: DW_TAG_member, name: "fp_offset", scope: !28, file: !1, line: 13, baseType: !31, size: 32, offset: 32)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "overflow_arg_area", scope: !28, file: !1, line: 13, baseType: !34, size: 64, offset: 64)
!34 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!35 = !DIDerivedType(tag: DW_TAG_member, name: "reg_save_area", scope: !28, file: !1, line: 13, baseType: !34, size: 64, offset: 128)
!36 = !{!37}
!37 = !DISubrange(count: 1)
!38 = !DILocation(line: 13, column: 11, scope: !10)
!39 = !DILocalVariable(name: "vl_count", scope: !10, file: !1, line: 13, type: !22)
!40 = !DILocation(line: 13, column: 15, scope: !10)
!41 = !DILocation(line: 14, column: 3, scope: !10)
!42 = !DILocation(line: 17, column: 3, scope: !10)
!43 = !DILocation(line: 18, column: 3, scope: !10)
!44 = !DILocation(line: 18, column: 14, scope: !45)
!45 = !DILexicalBlockFile(scope: !10, file: !1, discriminator: 1)
!46 = !DILocation(line: 18, column: 3, scope: !45)
!47 = !DILocation(line: 19, column: 11, scope: !48)
!48 = distinct !DILexicalBlock(scope: !10, file: !1, line: 18, column: 20)
!49 = !DILocation(line: 19, column: 11, scope: !50)
!50 = !DILexicalBlockFile(scope: !48, file: !1, discriminator: 1)
!51 = !DILocation(line: 19, column: 11, scope: !52)
!52 = !DILexicalBlockFile(scope: !48, file: !1, discriminator: 2)
!53 = !DILocation(line: 19, column: 11, scope: !54)
!54 = !DILexicalBlockFile(scope: !48, file: !1, discriminator: 3)
!55 = !DILocation(line: 20, column: 5, scope: !48)
!56 = !DILocation(line: 18, column: 3, scope: !57)
!57 = !DILexicalBlockFile(scope: !10, file: !1, discriminator: 2)
!58 = distinct !{!58, !43, !59}
!59 = !DILocation(line: 21, column: 3, scope: !10)
!60 = !DILocation(line: 22, column: 3, scope: !10)
!61 = !DILocation(line: 25, column: 43, scope: !10)
!62 = !DILocation(line: 25, column: 41, scope: !10)
!63 = !DILocation(line: 25, column: 49, scope: !10)
!64 = !DILocation(line: 25, column: 19, scope: !45)
!65 = !DILocalVariable(name: "buffer", scope: !10, file: !1, line: 9, type: !4)
!66 = !DILocation(line: 9, column: 9, scope: !10)
!67 = !DILocation(line: 26, column: 13, scope: !10)
!68 = !DILocation(line: 29, column: 3, scope: !10)
!69 = !DILocation(line: 29, column: 16, scope: !70)
!70 = !DILexicalBlockFile(scope: !71, file: !1, discriminator: 1)
!71 = distinct !DILexicalBlock(scope: !72, file: !1, line: 29, column: 3)
!72 = distinct !DILexicalBlock(scope: !10, file: !1, line: 29, column: 3)
!73 = !DILocation(line: 29, column: 3, scope: !74)
!74 = !DILexicalBlockFile(scope: !72, file: !1, discriminator: 1)
!75 = !DILocation(line: 30, column: 5, scope: !76)
!76 = distinct !DILexicalBlock(scope: !71, file: !1, line: 29, column: 30)
!77 = !DILocation(line: 29, column: 21, scope: !78)
!78 = !DILexicalBlockFile(scope: !71, file: !1, discriminator: 2)
!79 = !DILocation(line: 29, column: 3, scope: !78)
!80 = distinct !{!80, !81, !82}
!81 = !DILocation(line: 29, column: 3, scope: !72)
!82 = !DILocation(line: 31, column: 3, scope: !72)
!83 = !DILocation(line: 35, column: 3, scope: !10)
!84 = !DILocation(line: 37, column: 3, scope: !10)
!85 = !DILocation(line: 38, column: 1, scope: !10)
!86 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 40, type: !87, isLocal: false, isDefinition: true, scopeLine: 40, isOptimized: false, unit: !0, variables: !2)
!87 = !DISubroutineType(types: !88)
!88 = !{!13}
!89 = !DILocation(line: 41, column: 3, scope: !86)
!90 = !DILocation(line: 42, column: 3, scope: !86)
