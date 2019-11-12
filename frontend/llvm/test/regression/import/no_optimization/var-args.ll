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
define void @PrintInts(i32, ...) #0 !dbg !11 {
  %2 = alloca i32, align 4
  %3 = alloca i8*, align 8
  %4 = alloca i8*, align 8
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  %7 = alloca [1 x %struct.__va_list_tag], align 16
  %8 = alloca [1 x %struct.__va_list_tag], align 16
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.declare(metadata i8** %3, metadata !17, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.declare(metadata i8** %4, metadata !19, metadata !DIExpression()), !dbg !22
  %9 = getelementptr inbounds [6 x i8], [6 x i8]* @.str, i64 0, i64 0, !dbg !22
  store i8* %9, i8** %4, align 8, !dbg !22
  call void @llvm.dbg.declare(metadata i32* %5, metadata !23, metadata !DIExpression()), !dbg !24
  store i32 0, i32* %5, align 4, !dbg !24
  call void @llvm.dbg.declare(metadata i32* %6, metadata !25, metadata !DIExpression()), !dbg !26
  %10 = load i32, i32* %2, align 4, !dbg !27
  store i32 %10, i32* %6, align 4, !dbg !26
  call void @llvm.dbg.declare(metadata [1 x %struct.__va_list_tag]* %7, metadata !28, metadata !DIExpression()), !dbg !45
  call void @llvm.dbg.declare(metadata [1 x %struct.__va_list_tag]* %8, metadata !46, metadata !DIExpression()), !dbg !47
  %11 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %7, i64 0, i64 0, !dbg !48
  %12 = bitcast %struct.__va_list_tag* %11 to i8*, !dbg !48
  call void @llvm.va_start(i8* %12), !dbg !48
  %13 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %8, i64 0, i64 0, !dbg !49
  %14 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %7, i64 0, i64 0, !dbg !49
  %15 = bitcast %struct.__va_list_tag* %13 to i8*, !dbg !49
  %16 = bitcast %struct.__va_list_tag* %14 to i8*, !dbg !49
  call void @llvm.va_copy(i8* %15, i8* %16), !dbg !49
  br label %17, !dbg !50

17:                                               ; preds = %36, %1
  %18 = load i32, i32* %6, align 4, !dbg !51
  %19 = icmp ne i32 %18, 0, !dbg !52
  br i1 %19, label %20, label %41, !dbg !50

20:                                               ; preds = %17
  %21 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %8, i64 0, i64 0, !dbg !53
  %22 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %21, i32 0, i32 0, !dbg !53
  %23 = load i32, i32* %22, align 16, !dbg !53
  %24 = icmp ule i32 %23, 40, !dbg !53
  br i1 %24, label %25, label %31, !dbg !53

25:                                               ; preds = %20
  %26 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %21, i32 0, i32 3, !dbg !53
  %27 = load i8*, i8** %26, align 16, !dbg !53
  %28 = getelementptr i8, i8* %27, i32 %23, !dbg !53
  %29 = bitcast i8* %28 to i32*, !dbg !53
  %30 = add i32 %23, 8, !dbg !53
  store i32 %30, i32* %22, align 16, !dbg !53
  br label %36, !dbg !53

31:                                               ; preds = %20
  %32 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %21, i32 0, i32 2, !dbg !53
  %33 = load i8*, i8** %32, align 8, !dbg !53
  %34 = bitcast i8* %33 to i32*, !dbg !53
  %35 = getelementptr i8, i8* %33, i32 8, !dbg !53
  store i8* %35, i8** %32, align 8, !dbg !53
  br label %36, !dbg !53

36:                                               ; preds = %31, %25
  %37 = phi i32* [ %29, %25 ], [ %34, %31 ], !dbg !53
  %38 = load i32, i32* %37, align 4, !dbg !53
  store i32 %38, i32* %6, align 4, !dbg !55
  %39 = load i32, i32* %5, align 4, !dbg !56
  %40 = add nsw i32 %39, 1, !dbg !56
  store i32 %40, i32* %5, align 4, !dbg !56
  br label %17, !dbg !50, !llvm.loop !57

41:                                               ; preds = %17
  %42 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %8, i64 0, i64 0, !dbg !59
  %43 = bitcast %struct.__va_list_tag* %42 to i8*, !dbg !59
  call void @llvm.va_end(i8* %43), !dbg !59
  %44 = load i8*, i8** %4, align 8, !dbg !60
  %45 = call i64 @strlen(i8* %44), !dbg !61
  %46 = load i32, i32* %5, align 4, !dbg !62
  %47 = sext i32 %46 to i64, !dbg !62
  %48 = mul i64 %45, %47, !dbg !63
  %49 = add i64 %48, 1, !dbg !64
  %50 = call i8* @malloc(i64 %49) #5, !dbg !65
  store i8* %50, i8** %3, align 8, !dbg !66
  %51 = load i8*, i8** %3, align 8, !dbg !67
  %52 = getelementptr inbounds i8, i8* %51, i64 0, !dbg !67
  store i8 0, i8* %52, align 1, !dbg !68
  br label %53, !dbg !69

53:                                               ; preds = %60, %41
  %54 = load i32, i32* %5, align 4, !dbg !70
  %55 = icmp sgt i32 %54, 0, !dbg !73
  br i1 %55, label %56, label %63, !dbg !74

56:                                               ; preds = %53
  %57 = load i8*, i8** %3, align 8, !dbg !75
  %58 = load i8*, i8** %4, align 8, !dbg !77
  %59 = call i8* @strcat(i8* %57, i8* %58), !dbg !78
  br label %60, !dbg !79

60:                                               ; preds = %56
  %61 = load i32, i32* %5, align 4, !dbg !80
  %62 = add nsw i32 %61, -1, !dbg !80
  store i32 %62, i32* %5, align 4, !dbg !80
  br label %53, !dbg !81, !llvm.loop !82

63:                                               ; preds = %53
  %64 = load i8*, i8** %4, align 8, !dbg !84
  %65 = load i32, i32* %2, align 4, !dbg !85
  %66 = call i32 (i8*, ...) @printf(i8* %64, i32 %65), !dbg !86
  %67 = load i8*, i8** %3, align 8, !dbg !87
  %68 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %7, i64 0, i64 0, !dbg !88
  %69 = call i32 @vprintf(i8* %67, %struct.__va_list_tag* %68), !dbg !89
  %70 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %7, i64 0, i64 0, !dbg !90
  %71 = bitcast %struct.__va_list_tag* %70 to i8*, !dbg !90
  call void @llvm.va_end(i8* %71), !dbg !90
  ret void, !dbg !91
}
; CHECK: define void @PrintInts(si32 %1, ...) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32* $2 = allocate si32, 1, align 4
; CHECK:   si8** $3 = allocate si8*, 1, align 8
; CHECK:   si8** $4 = allocate si8*, 1, align 8
; CHECK:   si32* $5 = allocate si32, 1, align 4
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}]* $7 = allocate [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}], 1, align 16
; CHECK:   [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}]* $8 = allocate [1 x {0: ui32, 4: ui32, 8: si8*, 16: si8*}], 1, align 16
; CHECK:   store $2, %1, align 4
; CHECK:   si8* %9 = ptrshift @.str, 6 * 0, 1 * 0
; CHECK:   store $4, %9, align 8
; CHECK:   store $5, 0, align 4
; CHECK:   si32 %10 = load $2, align 4
; CHECK:   store $6, %10, align 4
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %11 = ptrshift $7, 24 * 0, 24 * 0
; CHECK:   si8* %12 = bitcast %11
; CHECK:   call @ar.va_start(%12)
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %13 = ptrshift $8, 24 * 0, 24 * 0
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %14 = ptrshift $7, 24 * 0, 24 * 0
; CHECK:   si8* %15 = bitcast %13
; CHECK:   si8* %16 = bitcast %14
; CHECK:   call @ar.va_copy(%15, %16)
; CHECK: }
; CHECK: #2 predecessors={#1, #8} successors={#3, #4} {
; CHECK:   si32 %17 = load $6, align 4
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#5, #6} {
; CHECK:   %17 sine 0
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %18 = ptrshift $8, 24 * 0, 24 * 0
; CHECK:   si32* %19 = ptrshift %18, 24 * 0, 1 * 0
; CHECK:   ui32* %20 = bitcast %19
; CHECK:   ui32 %21 = load %20, align 16
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#7} {
; CHECK:   %17 sieq 0
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %22 = ptrshift $8, 24 * 0, 24 * 0
; CHECK:   si8* %23 = bitcast %22
; CHECK:   call @ar.va_end(%23)
; CHECK:   si8* %24 = load $4, align 8
; CHECK:   ui64 %25 = call @ar.libc.strlen(%24)
; CHECK:   si32 %26 = load $5, align 4
; CHECK:   si64 %27 = sext %26
; CHECK:   ui64 %28 = bitcast %27
; CHECK:   ui64 %29 = %25 umul %28
; CHECK:   ui64 %30 = %29 uadd 1
; CHECK:   si8* %31 = call @ar.libc.malloc(%30)
; CHECK:   store $3, %31, align 8
; CHECK:   si8* %32 = load $3, align 8
; CHECK:   si8* %33 = ptrshift %32, 1 * 0
; CHECK:   store %33, 0, align 1
; CHECK: }
; CHECK: #5 predecessors={#3} successors={#8} {
; CHECK:   %21 uile 40
; CHECK:   si8** %34 = ptrshift %18, 24 * 0, 1 * 16
; CHECK:   si8* %35 = load %34, align 16
; CHECK:   si8* %36 = ptrshift %35, 1 * %21
; CHECK:   si32* %37 = bitcast %36
; CHECK:   ui32 %38 = %21 uadd 8
; CHECK:   si32 %39 = bitcast %38
; CHECK:   store %19, %39, align 16
; CHECK:   si32* %40 = %37
; CHECK: }
; CHECK: #6 predecessors={#3} successors={#8} {
; CHECK:   %21 uigt 40
; CHECK:   si8** %41 = ptrshift %18, 24 * 0, 1 * 8
; CHECK:   si8* %42 = load %41, align 8
; CHECK:   si32* %43 = bitcast %42
; CHECK:   si8* %44 = ptrshift %42, 1 * 8
; CHECK:   store %41, %44, align 8
; CHECK:   si32* %40 = %43
; CHECK: }
; CHECK: #7 predecessors={#4, #9} successors={#9, #10} {
; CHECK:   si32 %45 = load $5, align 4
; CHECK: }
; CHECK: #9 predecessors={#7} successors={#7} {
; CHECK:   %45 sigt 0
; CHECK:   si8* %49 = load $3, align 8
; CHECK:   si8* %50 = load $4, align 8
; CHECK:   si8* %51 = call @ar.libc.strcat(%49, %50)
; CHECK:   si32 %52 = load $5, align 4
; CHECK:   si32 %53 = %52 sadd.nw -1
; CHECK:   store $5, %53, align 4
; CHECK: }
; CHECK: #10 !exit predecessors={#7} {
; CHECK:   %45 sile 0
; CHECK:   si8* %54 = load $4, align 8
; CHECK:   si32 %55 = load $2, align 4
; CHECK:   si32 %56 = call @ar.libc.printf(%54, %55)
; CHECK:   si8* %57 = load $3, align 8
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %58 = ptrshift $7, 24 * 0, 24 * 0
; CHECK:   si32 %59 = call @vprintf(%57, %58)
; CHECK:   {0: si32, 4: si32, 8: si8*, 16: si8*}* %60 = ptrshift $7, 24 * 0, 24 * 0
; CHECK:   si8* %61 = bitcast %60
; CHECK:   call @ar.va_end(%61)
; CHECK:   return
; CHECK: }
; CHECK: #8 predecessors={#5, #6} successors={#2} {
; CHECK:   si32 %46 = load %40, align 4
; CHECK:   store $6, %46, align 4
; CHECK:   si32 %47 = load $5, align 4
; CHECK:   si32 %48 = %47 sadd.nw 1
; CHECK:   store $5, %48, align 4
; CHECK: }
; CHECK: }

; Function Attrs: allocsize(0)
declare i8* @malloc(i64) #3
; CHECK: declare si8* @ar.libc.malloc(ui64)

declare i32 @printf(i8*, ...) #4
; CHECK: declare si32 @ar.libc.printf(si8*, ...)

declare i8* @strcat(i8*, i8*) #4
; CHECK: declare si8* @ar.libc.strcat(si8*, si8*)

declare i64 @strlen(i8*) #4
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

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !92 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  call void (i32, ...) @PrintInts(i32 10, i32 20, i32 30, i32 40, i32 50, i32 0), !dbg !95
  ret i32 0, !dbg !96
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   store $1, 0, align 4
; CHECK:   call @PrintInts(10, 20, 30, 40, 50, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

declare i32 @vprintf(i8*, %struct.__va_list_tag*) #4
; CHECK: declare si32 @vprintf(si8*, {0: si32, 4: si32, 8: si8*, 16: si8*}*)

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }
attributes #3 = { allocsize(0) "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { allocsize(0) }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!6, !7, !8, !9}
!llvm.ident = !{!10}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "var-args.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!16 = !DILocation(line: 8, column: 20, scope: !11)
!17 = !DILocalVariable(name: "buffer", scope: !11, file: !1, line: 9, type: !4)
!18 = !DILocation(line: 9, column: 9, scope: !11)
!19 = !DILocalVariable(name: "format", scope: !11, file: !1, line: 10, type: !20)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !21, size: 64)
!21 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !5)
!22 = !DILocation(line: 10, column: 15, scope: !11)
!23 = !DILocalVariable(name: "count", scope: !11, file: !1, line: 11, type: !14)
!24 = !DILocation(line: 11, column: 7, scope: !11)
!25 = !DILocalVariable(name: "val", scope: !11, file: !1, line: 12, type: !14)
!26 = !DILocation(line: 12, column: 7, scope: !11)
!27 = !DILocation(line: 12, column: 13, scope: !11)
!28 = !DILocalVariable(name: "vl", scope: !11, file: !1, line: 13, type: !29)
!29 = !DIDerivedType(tag: DW_TAG_typedef, name: "va_list", file: !30, line: 32, baseType: !31)
!30 = !DIFile(filename: "/usr/include/sys/_types/_va_list.h", directory: "")
!31 = !DIDerivedType(tag: DW_TAG_typedef, name: "__darwin_va_list", file: !32, line: 98, baseType: !33)
!32 = !DIFile(filename: "/usr/include/i386/_types.h", directory: "")
!33 = !DIDerivedType(tag: DW_TAG_typedef, name: "__builtin_va_list", file: !1, line: 13, baseType: !34)
!34 = !DICompositeType(tag: DW_TAG_array_type, baseType: !35, size: 192, elements: !43)
!35 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__va_list_tag", file: !1, line: 13, size: 192, elements: !36)
!36 = !{!37, !39, !40, !42}
!37 = !DIDerivedType(tag: DW_TAG_member, name: "gp_offset", scope: !35, file: !1, line: 13, baseType: !38, size: 32)
!38 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!39 = !DIDerivedType(tag: DW_TAG_member, name: "fp_offset", scope: !35, file: !1, line: 13, baseType: !38, size: 32, offset: 32)
!40 = !DIDerivedType(tag: DW_TAG_member, name: "overflow_arg_area", scope: !35, file: !1, line: 13, baseType: !41, size: 64, offset: 64)
!41 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!42 = !DIDerivedType(tag: DW_TAG_member, name: "reg_save_area", scope: !35, file: !1, line: 13, baseType: !41, size: 64, offset: 128)
!43 = !{!44}
!44 = !DISubrange(count: 1)
!45 = !DILocation(line: 13, column: 11, scope: !11)
!46 = !DILocalVariable(name: "vl_count", scope: !11, file: !1, line: 13, type: !29)
!47 = !DILocation(line: 13, column: 15, scope: !11)
!48 = !DILocation(line: 14, column: 3, scope: !11)
!49 = !DILocation(line: 17, column: 3, scope: !11)
!50 = !DILocation(line: 18, column: 3, scope: !11)
!51 = !DILocation(line: 18, column: 10, scope: !11)
!52 = !DILocation(line: 18, column: 14, scope: !11)
!53 = !DILocation(line: 19, column: 11, scope: !54)
!54 = distinct !DILexicalBlock(scope: !11, file: !1, line: 18, column: 20)
!55 = !DILocation(line: 19, column: 9, scope: !54)
!56 = !DILocation(line: 20, column: 5, scope: !54)
!57 = distinct !{!57, !50, !58}
!58 = !DILocation(line: 21, column: 3, scope: !11)
!59 = !DILocation(line: 22, column: 3, scope: !11)
!60 = !DILocation(line: 25, column: 33, scope: !11)
!61 = !DILocation(line: 25, column: 26, scope: !11)
!62 = !DILocation(line: 25, column: 43, scope: !11)
!63 = !DILocation(line: 25, column: 41, scope: !11)
!64 = !DILocation(line: 25, column: 49, scope: !11)
!65 = !DILocation(line: 25, column: 19, scope: !11)
!66 = !DILocation(line: 25, column: 10, scope: !11)
!67 = !DILocation(line: 26, column: 3, scope: !11)
!68 = !DILocation(line: 26, column: 13, scope: !11)
!69 = !DILocation(line: 29, column: 3, scope: !11)
!70 = !DILocation(line: 29, column: 10, scope: !71)
!71 = distinct !DILexicalBlock(scope: !72, file: !1, line: 29, column: 3)
!72 = distinct !DILexicalBlock(scope: !11, file: !1, line: 29, column: 3)
!73 = !DILocation(line: 29, column: 16, scope: !71)
!74 = !DILocation(line: 29, column: 3, scope: !72)
!75 = !DILocation(line: 30, column: 12, scope: !76)
!76 = distinct !DILexicalBlock(scope: !71, file: !1, line: 29, column: 30)
!77 = !DILocation(line: 30, column: 20, scope: !76)
!78 = !DILocation(line: 30, column: 5, scope: !76)
!79 = !DILocation(line: 31, column: 3, scope: !76)
!80 = !DILocation(line: 29, column: 21, scope: !71)
!81 = !DILocation(line: 29, column: 3, scope: !71)
!82 = distinct !{!82, !74, !83}
!83 = !DILocation(line: 31, column: 3, scope: !72)
!84 = !DILocation(line: 34, column: 10, scope: !11)
!85 = !DILocation(line: 34, column: 18, scope: !11)
!86 = !DILocation(line: 34, column: 3, scope: !11)
!87 = !DILocation(line: 35, column: 11, scope: !11)
!88 = !DILocation(line: 35, column: 19, scope: !11)
!89 = !DILocation(line: 35, column: 3, scope: !11)
!90 = !DILocation(line: 37, column: 3, scope: !11)
!91 = !DILocation(line: 38, column: 1, scope: !11)
!92 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 40, type: !93, scopeLine: 40, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!93 = !DISubroutineType(types: !94)
!94 = !{!14}
!95 = !DILocation(line: 41, column: 3, scope: !92)
!96 = !DILocation(line: 42, column: 3, scope: !92)
