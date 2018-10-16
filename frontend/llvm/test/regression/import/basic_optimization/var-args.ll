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
define void @PrintInts(i32, ...) #0 !dbg !10 {
  %2 = alloca [1 x %struct.__va_list_tag], align 16
  %3 = alloca [1 x %struct.__va_list_tag], align 16
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !14, metadata !15), !dbg !16
  call void @llvm.dbg.value(metadata i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.str, i32 0, i32 0), i64 0, metadata !17, metadata !15), !dbg !20
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !21, metadata !15), !dbg !22
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !23, metadata !15), !dbg !24
  call void @llvm.dbg.declare(metadata [1 x %struct.__va_list_tag]* %2, metadata !25, metadata !15), !dbg !42
  call void @llvm.dbg.declare(metadata [1 x %struct.__va_list_tag]* %3, metadata !43, metadata !15), !dbg !44
  %4 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %2, i32 0, i32 0, !dbg !45
  %5 = bitcast %struct.__va_list_tag* %4 to i8*, !dbg !45
  call void @llvm.va_start(i8* %5), !dbg !45
  %6 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %3, i32 0, i32 0, !dbg !46
  %7 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %2, i32 0, i32 0, !dbg !46
  %8 = bitcast %struct.__va_list_tag* %6 to i8*, !dbg !46
  %9 = bitcast %struct.__va_list_tag* %7 to i8*, !dbg !46
  call void @llvm.va_copy(i8* %8, i8* %9), !dbg !46
  br label %10, !dbg !47

; <label>:10:                                     ; preds = %28, %1
  %.01 = phi i32 [ 0, %1 ], [ %31, %28 ]
  %.0 = phi i32 [ %0, %1 ], [ %30, %28 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !23, metadata !15), !dbg !24
  call void @llvm.dbg.value(metadata i32 %.01, i64 0, metadata !21, metadata !15), !dbg !22
  %11 = icmp ne i32 %.0, 0, !dbg !48
  br i1 %11, label %12, label %32, !dbg !50

; <label>:12:                                     ; preds = %10
  %13 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %3, i32 0, i32 0, !dbg !51
  %14 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %13, i32 0, i32 0, !dbg !51
  %15 = load i32, i32* %14, align 16, !dbg !51
  %16 = icmp ule i32 %15, 40, !dbg !51
  br i1 %16, label %17, label %23, !dbg !51

; <label>:17:                                     ; preds = %12
  %18 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %13, i32 0, i32 3, !dbg !53
  %19 = load i8*, i8** %18, align 16, !dbg !53
  %20 = getelementptr i8, i8* %19, i32 %15, !dbg !53
  %21 = bitcast i8* %20 to i32*, !dbg !53
  %22 = add i32 %15, 8, !dbg !53
  store i32 %22, i32* %14, align 16, !dbg !53
  br label %28, !dbg !53

; <label>:23:                                     ; preds = %12
  %24 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %13, i32 0, i32 2, !dbg !55
  %25 = load i8*, i8** %24, align 8, !dbg !55
  %26 = bitcast i8* %25 to i32*, !dbg !55
  %27 = getelementptr i8, i8* %25, i32 8, !dbg !55
  store i8* %27, i8** %24, align 8, !dbg !55
  br label %28, !dbg !55

; <label>:28:                                     ; preds = %23, %17
  %29 = phi i32* [ %21, %17 ], [ %26, %23 ], !dbg !57
  %30 = load i32, i32* %29, align 4, !dbg !57
  call void @llvm.dbg.value(metadata i32 %30, i64 0, metadata !23, metadata !15), !dbg !24
  %31 = add nsw i32 %.01, 1, !dbg !59
  call void @llvm.dbg.value(metadata i32 %31, i64 0, metadata !21, metadata !15), !dbg !22
  br label %10, !dbg !60, !llvm.loop !62

; <label>:32:                                     ; preds = %10
  %33 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %3, i32 0, i32 0, !dbg !64
  %34 = bitcast %struct.__va_list_tag* %33 to i8*, !dbg !64
  call void @llvm.va_end(i8* %34), !dbg !64
  %35 = getelementptr inbounds [6 x i8], [6 x i8]* @.str, i32 0, i32 0, !dbg !65
  %36 = call i64 @strlen(i8* %35), !dbg !65
  %37 = sext i32 %.01 to i64, !dbg !66
  %38 = mul i64 %36, %37, !dbg !67
  %39 = add i64 %38, 1, !dbg !68
  %40 = call i8* @malloc(i64 %39), !dbg !69
  call void @llvm.dbg.value(metadata i8* %40, i64 0, metadata !70, metadata !15), !dbg !71
  %41 = getelementptr inbounds i8, i8* %40, i64 0, !dbg !72
  store i8 0, i8* %41, align 1, !dbg !73
  br label %42, !dbg !74

; <label>:42:                                     ; preds = %47, %32
  %.1 = phi i32 [ %.01, %32 ], [ %48, %47 ]
  call void @llvm.dbg.value(metadata i32 %.1, i64 0, metadata !21, metadata !15), !dbg !22
  %43 = icmp sgt i32 %.1, 0, !dbg !75
  br i1 %43, label %44, label %49, !dbg !79

; <label>:44:                                     ; preds = %42
  %45 = getelementptr inbounds [6 x i8], [6 x i8]* @.str, i32 0, i32 0, !dbg !81
  %46 = call i8* @strcat(i8* %40, i8* %45), !dbg !81
  br label %47, !dbg !83

; <label>:47:                                     ; preds = %44
  %48 = add nsw i32 %.1, -1, !dbg !84
  call void @llvm.dbg.value(metadata i32 %48, i64 0, metadata !21, metadata !15), !dbg !22
  br label %42, !dbg !86, !llvm.loop !87

; <label>:49:                                     ; preds = %42
  %50 = getelementptr inbounds [6 x i8], [6 x i8]* @.str, i32 0, i32 0, !dbg !90
  %51 = call i32 (i8*, ...) @printf(i8* %50, i32 %0), !dbg !90
  %52 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %2, i32 0, i32 0, !dbg !91
  %53 = call i32 @vprintf(i8* %40, %struct.__va_list_tag* %52), !dbg !92
  %54 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %2, i32 0, i32 0, !dbg !93
  %55 = bitcast %struct.__va_list_tag* %54 to i8*, !dbg !93
  call void @llvm.va_end(i8* %55), !dbg !93
  ret void, !dbg !94
}
; CHECK: define void @PrintInts(si32 %1, ...) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}]* $2 = allocate [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}], 1, align 16
; CHECK:   [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}]* $3 = allocate [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}], 1, align 16
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %4 = ptrshift $2, 24 * 0, 24 * 0
; CHECK:   si8* %5 = bitcast %4
; CHECK:   call @ar.va_start(%5)
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %6 = ptrshift $3, 24 * 0, 24 * 0
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %7 = ptrshift $2, 24 * 0, 24 * 0
; CHECK:   si8* %8 = bitcast %6
; CHECK:   si8* %9 = bitcast %7
; CHECK:   call @ar.va_copy(%8, %9)
; CHECK:   si32 %.01 = 0
; CHECK:   si32 %.0 = %1
; CHECK: }
; CHECK: #2 predecessors={#1, #8} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#5, #6} {
; CHECK:   %.0 sine 0
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %10 = ptrshift $3, 24 * 0, 24 * 0
; CHECK:   si32* %11 = ptrshift %10, 24 * 0, 1 * 0
; CHECK:   ui32* %12 = bitcast %11
; CHECK:   ui32 %13 = load %12, align 16
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#7} {
; CHECK:   %.0 sieq 0
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %14 = ptrshift $3, 24 * 0, 24 * 0
; CHECK:   si8* %15 = bitcast %14
; CHECK:   call @ar.va_end(%15)
; CHECK:   si8* %16 = ptrshift @.str, 6 * 0, 1 * 0
; CHECK:   ui64 %17 = call @ar.libc.strlen(%16)
; CHECK:   si64 %18 = sext %.01
; CHECK:   ui64 %19 = bitcast %18
; CHECK:   ui64 %20 = %17 umul %19
; CHECK:   ui64 %21 = %20 uadd 1
; CHECK:   si8* %22 = call @ar.libc.malloc(%21)
; CHECK:   si8* %23 = ptrshift %22, 1 * 0
; CHECK:   store %23, 0, align 1
; CHECK:   si32 %.1 = %.01
; CHECK: }
; CHECK: #5 predecessors={#3} successors={#8} {
; CHECK:   %13 uile 40
; CHECK:   si8** %24 = ptrshift %10, 24 * 0, 1 * 16
; CHECK:   si8* %25 = load %24, align 16
; CHECK:   si8* %26 = ptrshift %25, 1 * %13
; CHECK:   si32* %27 = bitcast %26
; CHECK:   ui32 %28 = %13 uadd 8
; CHECK:   si32 %29 = bitcast %28
; CHECK:   store %11, %29, align 16
; CHECK:   si32* %30 = %27
; CHECK: }
; CHECK: #6 predecessors={#3} successors={#8} {
; CHECK:   %13 uigt 40
; CHECK:   si8** %31 = ptrshift %10, 24 * 0, 1 * 8
; CHECK:   si8* %32 = load %31, align 8
; CHECK:   si32* %33 = bitcast %32
; CHECK:   si8* %34 = ptrshift %32, 1 * 8
; CHECK:   store %31, %34, align 8
; CHECK:   si32* %30 = %33
; CHECK: }
; CHECK: #7 predecessors={#4, #9} successors={#9, #10} {
; CHECK: }
; CHECK: #9 predecessors={#7} successors={#7} {
; CHECK:   %.1 sigt 0
; CHECK:   si8* %37 = ptrshift @.str, 6 * 0, 1 * 0
; CHECK:   si8* %38 = call @ar.libc.strcat(%22, %37)
; CHECK:   si32 %39 = %.1 sadd.nw -1
; CHECK:   si32 %.1 = %39
; CHECK: }
; CHECK: #10 !exit predecessors={#7} {
; CHECK:   %.1 sile 0
; CHECK:   si8* %40 = ptrshift @.str, 6 * 0, 1 * 0
; CHECK:   si32 %41 = call @ar.libc.printf(%40, %1)
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %42 = ptrshift $2, 24 * 0, 24 * 0
; CHECK:   si32 %43 = call @vprintf(%22, %42)
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %44 = ptrshift $2, 24 * 0, 24 * 0
; CHECK:   si8* %45 = bitcast %44
; CHECK:   call @ar.va_end(%45)
; CHECK:   return
; CHECK: }
; CHECK: #8 predecessors={#5, #6} successors={#2} {
; CHECK:   si32 %35 = load %30, align 4
; CHECK:   si32 %36 = %.01 sadd.nw 1
; CHECK:   si32 %.01 = %36
; CHECK:   si32 %.0 = %35
; CHECK: }
; CHECK: }

declare i8* @malloc(i64) #3
; CHECK: declare si8* @ar.libc.malloc(ui64)

declare i32 @printf(i8*, ...) #3
; CHECK: declare si32 @ar.libc.printf(si8*, ...)

declare i8* @strcat(i8*, i8*) #3
; CHECK: declare si8* @ar.libc.strcat(si8*, si8*)

declare i64 @strlen(i8*) #3
; CHECK: declare ui64 @ar.libc.strlen(si8*)

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
define i32 @main() #0 !dbg !95 {
  call void (i32, ...) @PrintInts(i32 10, i32 20, i32 30, i32 40, i32 50, i32 0), !dbg !98
  ret i32 0, !dbg !99
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   call @PrintInts(10, 20, 30, 40, 50, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

declare i32 @vprintf(i8*, %struct.__va_list_tag*) #3
; CHECK: declare si32 @vprintf(si8*, {0: si32, 4: si32, 8: si8*, 16: si8*}*)

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3)
!1 = !DIFile(filename: "var-args.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
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
!17 = !DILocalVariable(name: "format", scope: !10, file: !1, line: 10, type: !18)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!19 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !5)
!20 = !DILocation(line: 10, column: 15, scope: !10)
!21 = !DILocalVariable(name: "count", scope: !10, file: !1, line: 11, type: !13)
!22 = !DILocation(line: 11, column: 7, scope: !10)
!23 = !DILocalVariable(name: "val", scope: !10, file: !1, line: 12, type: !13)
!24 = !DILocation(line: 12, column: 7, scope: !10)
!25 = !DILocalVariable(name: "vl", scope: !10, file: !1, line: 13, type: !26)
!26 = !DIDerivedType(tag: DW_TAG_typedef, name: "va_list", file: !27, line: 31, baseType: !28)
!27 = !DIFile(filename: "/usr/include/sys/_types/_va_list.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
!28 = !DIDerivedType(tag: DW_TAG_typedef, name: "__darwin_va_list", file: !29, line: 98, baseType: !30)
!29 = !DIFile(filename: "/usr/include/i386/_types.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
!30 = !DIDerivedType(tag: DW_TAG_typedef, name: "__builtin_va_list", file: !1, line: 13, baseType: !31)
!31 = !DICompositeType(tag: DW_TAG_array_type, baseType: !32, size: 192, elements: !40)
!32 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__va_list_tag", file: !1, line: 13, size: 192, elements: !33)
!33 = !{!34, !36, !37, !39}
!34 = !DIDerivedType(tag: DW_TAG_member, name: "gp_offset", scope: !32, file: !1, line: 13, baseType: !35, size: 32)
!35 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!36 = !DIDerivedType(tag: DW_TAG_member, name: "fp_offset", scope: !32, file: !1, line: 13, baseType: !35, size: 32, offset: 32)
!37 = !DIDerivedType(tag: DW_TAG_member, name: "overflow_arg_area", scope: !32, file: !1, line: 13, baseType: !38, size: 64, offset: 64)
!38 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!39 = !DIDerivedType(tag: DW_TAG_member, name: "reg_save_area", scope: !32, file: !1, line: 13, baseType: !38, size: 64, offset: 128)
!40 = !{!41}
!41 = !DISubrange(count: 1)
!42 = !DILocation(line: 13, column: 11, scope: !10)
!43 = !DILocalVariable(name: "vl_count", scope: !10, file: !1, line: 13, type: !26)
!44 = !DILocation(line: 13, column: 15, scope: !10)
!45 = !DILocation(line: 14, column: 3, scope: !10)
!46 = !DILocation(line: 17, column: 3, scope: !10)
!47 = !DILocation(line: 18, column: 3, scope: !10)
!48 = !DILocation(line: 18, column: 14, scope: !49)
!49 = !DILexicalBlockFile(scope: !10, file: !1, discriminator: 1)
!50 = !DILocation(line: 18, column: 3, scope: !49)
!51 = !DILocation(line: 19, column: 11, scope: !52)
!52 = distinct !DILexicalBlock(scope: !10, file: !1, line: 18, column: 20)
!53 = !DILocation(line: 19, column: 11, scope: !54)
!54 = !DILexicalBlockFile(scope: !52, file: !1, discriminator: 1)
!55 = !DILocation(line: 19, column: 11, scope: !56)
!56 = !DILexicalBlockFile(scope: !52, file: !1, discriminator: 2)
!57 = !DILocation(line: 19, column: 11, scope: !58)
!58 = !DILexicalBlockFile(scope: !52, file: !1, discriminator: 3)
!59 = !DILocation(line: 20, column: 5, scope: !52)
!60 = !DILocation(line: 18, column: 3, scope: !61)
!61 = !DILexicalBlockFile(scope: !10, file: !1, discriminator: 2)
!62 = distinct !{!62, !47, !63}
!63 = !DILocation(line: 21, column: 3, scope: !10)
!64 = !DILocation(line: 22, column: 3, scope: !10)
!65 = !DILocation(line: 25, column: 26, scope: !10)
!66 = !DILocation(line: 25, column: 43, scope: !10)
!67 = !DILocation(line: 25, column: 41, scope: !10)
!68 = !DILocation(line: 25, column: 49, scope: !10)
!69 = !DILocation(line: 25, column: 19, scope: !49)
!70 = !DILocalVariable(name: "buffer", scope: !10, file: !1, line: 9, type: !4)
!71 = !DILocation(line: 9, column: 9, scope: !10)
!72 = !DILocation(line: 26, column: 3, scope: !10)
!73 = !DILocation(line: 26, column: 13, scope: !10)
!74 = !DILocation(line: 29, column: 3, scope: !10)
!75 = !DILocation(line: 29, column: 16, scope: !76)
!76 = !DILexicalBlockFile(scope: !77, file: !1, discriminator: 1)
!77 = distinct !DILexicalBlock(scope: !78, file: !1, line: 29, column: 3)
!78 = distinct !DILexicalBlock(scope: !10, file: !1, line: 29, column: 3)
!79 = !DILocation(line: 29, column: 3, scope: !80)
!80 = !DILexicalBlockFile(scope: !78, file: !1, discriminator: 1)
!81 = !DILocation(line: 30, column: 5, scope: !82)
!82 = distinct !DILexicalBlock(scope: !77, file: !1, line: 29, column: 30)
!83 = !DILocation(line: 31, column: 3, scope: !82)
!84 = !DILocation(line: 29, column: 21, scope: !85)
!85 = !DILexicalBlockFile(scope: !77, file: !1, discriminator: 2)
!86 = !DILocation(line: 29, column: 3, scope: !85)
!87 = distinct !{!87, !88, !89}
!88 = !DILocation(line: 29, column: 3, scope: !78)
!89 = !DILocation(line: 31, column: 3, scope: !78)
!90 = !DILocation(line: 34, column: 3, scope: !10)
!91 = !DILocation(line: 35, column: 19, scope: !10)
!92 = !DILocation(line: 35, column: 3, scope: !10)
!93 = !DILocation(line: 37, column: 3, scope: !10)
!94 = !DILocation(line: 38, column: 1, scope: !10)
!95 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 40, type: !96, isLocal: false, isDefinition: true, scopeLine: 40, isOptimized: false, unit: !0, variables: !2)
!96 = !DISubroutineType(types: !97)
!97 = !{!13}
!98 = !DILocation(line: 41, column: 3, scope: !95)
!99 = !DILocation(line: 42, column: 3, scope: !95)
