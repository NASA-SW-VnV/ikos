; ModuleID = 'file-intrinsics.pp.bc'
source_filename = "file-intrinsics.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.__sFILE = type { i8*, i32, i32, i16, i16, %struct.__sbuf, i32, i8*, i32 (i8*)*, i32 (i8*, i8*, i32)*, i64 (i8*, i64, i32)*, i32 (i8*, i8*, i32)*, %struct.__sbuf, %struct.__sFILEX*, i32, [3 x i8], [1 x i8], %struct.__sbuf, i32, i64 }
%struct.__sFILEX = type opaque
%struct.__sbuf = type { i8*, i32 }

@.str = private unnamed_addr constant [10 x i8] c"/tmp/test\00", align 1
; CHECK: define [10 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [47, 116, 109, 112, 47, 116, 101, 115, 116, 0], align 1
; CHECK: }
; CHECK: }

@.str.1 = private unnamed_addr constant [3 x i8] c"rw\00", align 1
; CHECK: define [3 x si8]* @.str.1, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str.1, [114, 119, 0], align 1
; CHECK: }
; CHECK: }

@.str.2 = private unnamed_addr constant [12 x i8] c"hello world\00", align 1
; CHECK: define [12 x si8]* @.str.2, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str.2, [104, 101, 108, 108, 111, 32, 119, 111, 114, 108, 100, 0], align 1
; CHECK: }
; CHECK: }

@.str.3 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
; CHECK: define [3 x si8]* @.str.3, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str.3, [37, 100, 0], align 1
; CHECK: }
; CHECK: }

declare i32 @fclose(%struct.__sFILE*) local_unnamed_addr #2
; CHECK: declare si32 @ar.libc.fclose(opaque*)

declare i32 @fflush(%struct.__sFILE*) local_unnamed_addr #2
; CHECK: declare si32 @ar.libc.fflush(opaque*)

declare i32 @fgetc(%struct.__sFILE*) local_unnamed_addr #2
; CHECK: declare si32 @ar.libc.fgetc(opaque*)

declare i8* @fgets(i8*, i32, %struct.__sFILE*) local_unnamed_addr #2
; CHECK: declare si8* @ar.libc.fgets(si8*, si32, opaque*)

declare %struct.__sFILE* @"\01_fopen"(i8*, i8*) local_unnamed_addr #2
; CHECK: declare opaque* @ar.libc.fopen(si8*, si8*)

declare i32 @"\01_fputs"(i8*, %struct.__sFILE*) local_unnamed_addr #2
; CHECK: declare si32 @ar.libc.fputs(si8*, opaque*)

declare i32 @fscanf(%struct.__sFILE*, i8*, ...) local_unnamed_addr #2
; CHECK: declare si32 @ar.libc.fscanf(opaque*, si8*, ...)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() local_unnamed_addr #0 !dbg !8 {
  %1 = alloca [1025 x i8], align 16
  %2 = alloca i32, align 4
  %3 = getelementptr inbounds [10 x i8], [10 x i8]* @.str, i64 0, i64 0, !dbg !12
  %4 = getelementptr inbounds [3 x i8], [3 x i8]* @.str.1, i64 0, i64 0, !dbg !12
  %5 = call %struct.__sFILE* @"\01_fopen"(i8* %3, i8* %4) #3, !dbg !12
  call void @llvm.dbg.value(metadata %struct.__sFILE* %5, metadata !13, metadata !DIExpression()), !dbg !77
  call void @llvm.dbg.declare(metadata [1025 x i8]* %1, metadata !78, metadata !DIExpression()), !dbg !82
  %6 = getelementptr inbounds [1025 x i8], [1025 x i8]* %1, i64 0, i64 0, !dbg !83
  %7 = call i8* @fgets(i8* nonnull %6, i32 1024, %struct.__sFILE* %5) #3, !dbg !84
  %8 = call i32 @fgetc(%struct.__sFILE* %5) #3, !dbg !85
  %9 = getelementptr inbounds [12 x i8], [12 x i8]* @.str.2, i64 0, i64 0, !dbg !86
  %10 = call i32 @"\01_fputs"(i8* %9, %struct.__sFILE* %5) #3, !dbg !86
  call void @llvm.dbg.value(metadata i32* %2, metadata !87, metadata !DIExpression(DW_OP_deref)), !dbg !77
  %11 = getelementptr inbounds [3 x i8], [3 x i8]* @.str.3, i64 0, i64 0, !dbg !88
  %12 = call i32 (%struct.__sFILE*, i8*, ...) @fscanf(%struct.__sFILE* %5, i8* %11, i32* nonnull %2) #3, !dbg !88
  %13 = call i32 @fflush(%struct.__sFILE* %5) #3, !dbg !89
  %14 = call i32 @fclose(%struct.__sFILE* %5) #3, !dbg !90
  ret i32 0, !dbg !91
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   [1025 x si8]* $1 = allocate [1025 x si8], 1, align 16
; CHECK:   si32* $2 = allocate si32, 1, align 4
; CHECK:   si8* %3 = ptrshift @.str, 10 * 0, 1 * 0
; CHECK:   si8* %4 = ptrshift @.str.1, 3 * 0, 1 * 0
; CHECK:   opaque* %5 = call @ar.libc.fopen(%3, %4)
; CHECK:   {0: ui8*, 8: si32, 12: si32, 16: si16, 18: si16, 24: {0: ui8*, 8: si32}, 40: si32, 48: si8*, 56: si32 (si8*)*, 64: si32 (si8*, si8*, si32)*, 72: si64 (si8*, si64, si32)*, 80: si32 (si8*, si8*, si32)*, 88: {0: ui8*, 8: si32}, 104: opaque*, 112: si32, 116: [3 x ui8], 119: [1 x ui8], 120: {0: ui8*, 8: si32}, 136: si32, 144: si64}* %6 = bitcast %5
; CHECK:   si8* %7 = ptrshift $1, 1025 * 0, 1 * 0
; CHECK:   opaque* %8 = bitcast %6
; CHECK:   si8* %9 = call @ar.libc.fgets(%7, 1024, %8)
; CHECK:   opaque* %10 = bitcast %6
; CHECK:   si32 %11 = call @ar.libc.fgetc(%10)
; CHECK:   si8* %12 = ptrshift @.str.2, 12 * 0, 1 * 0
; CHECK:   opaque* %13 = bitcast %6
; CHECK:   si32 %14 = call @ar.libc.fputs(%12, %13)
; CHECK:   si8* %15 = ptrshift @.str.3, 3 * 0, 1 * 0
; CHECK:   opaque* %16 = bitcast %6
; CHECK:   si32 %17 = call @ar.libc.fscanf(%16, %15, $2)
; CHECK:   opaque* %18 = bitcast %6
; CHECK:   si32 %19 = call @ar.libc.fflush(%18)
; CHECK:   opaque* %20 = bitcast %6
; CHECK:   si32 %21 = call @ar.libc.fclose(%20)
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
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "file-intrinsics.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocation(line: 4, column: 13, scope: !8)
!13 = !DILocalVariable(name: "f", scope: !8, file: !1, line: 4, type: !14)
!14 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !15, size: 64)
!15 = !DIDerivedType(tag: DW_TAG_typedef, name: "FILE", file: !16, line: 157, baseType: !17)
!16 = !DIFile(filename: "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/_stdio.h", directory: "")
!17 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__sFILE", file: !16, line: 126, size: 1216, elements: !18)
!18 = !{!19, !22, !23, !24, !26, !27, !32, !33, !35, !39, !45, !55, !61, !62, !65, !66, !70, !74, !75, !76}
!19 = !DIDerivedType(tag: DW_TAG_member, name: "_p", scope: !17, file: !16, line: 127, baseType: !20, size: 64)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !21, size: 64)
!21 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!22 = !DIDerivedType(tag: DW_TAG_member, name: "_r", scope: !17, file: !16, line: 128, baseType: !11, size: 32, offset: 64)
!23 = !DIDerivedType(tag: DW_TAG_member, name: "_w", scope: !17, file: !16, line: 129, baseType: !11, size: 32, offset: 96)
!24 = !DIDerivedType(tag: DW_TAG_member, name: "_flags", scope: !17, file: !16, line: 130, baseType: !25, size: 16, offset: 128)
!25 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!26 = !DIDerivedType(tag: DW_TAG_member, name: "_file", scope: !17, file: !16, line: 131, baseType: !25, size: 16, offset: 144)
!27 = !DIDerivedType(tag: DW_TAG_member, name: "_bf", scope: !17, file: !16, line: 132, baseType: !28, size: 128, offset: 192)
!28 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__sbuf", file: !16, line: 92, size: 128, elements: !29)
!29 = !{!30, !31}
!30 = !DIDerivedType(tag: DW_TAG_member, name: "_base", scope: !28, file: !16, line: 93, baseType: !20, size: 64)
!31 = !DIDerivedType(tag: DW_TAG_member, name: "_size", scope: !28, file: !16, line: 94, baseType: !11, size: 32, offset: 64)
!32 = !DIDerivedType(tag: DW_TAG_member, name: "_lbfsize", scope: !17, file: !16, line: 133, baseType: !11, size: 32, offset: 320)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "_cookie", scope: !17, file: !16, line: 136, baseType: !34, size: 64, offset: 384)
!34 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!35 = !DIDerivedType(tag: DW_TAG_member, name: "_close", scope: !17, file: !16, line: 137, baseType: !36, size: 64, offset: 448)
!36 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !37, size: 64)
!37 = !DISubroutineType(types: !38)
!38 = !{!11, !34}
!39 = !DIDerivedType(tag: DW_TAG_member, name: "_read", scope: !17, file: !16, line: 138, baseType: !40, size: 64, offset: 512)
!40 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !41, size: 64)
!41 = !DISubroutineType(types: !42)
!42 = !{!11, !34, !43, !11}
!43 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !44, size: 64)
!44 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!45 = !DIDerivedType(tag: DW_TAG_member, name: "_seek", scope: !17, file: !16, line: 139, baseType: !46, size: 64, offset: 576)
!46 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !47, size: 64)
!47 = !DISubroutineType(types: !48)
!48 = !{!49, !34, !49, !11}
!49 = !DIDerivedType(tag: DW_TAG_typedef, name: "fpos_t", file: !16, line: 81, baseType: !50)
!50 = !DIDerivedType(tag: DW_TAG_typedef, name: "__darwin_off_t", file: !51, line: 71, baseType: !52)
!51 = !DIFile(filename: "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/sys/_types.h", directory: "")
!52 = !DIDerivedType(tag: DW_TAG_typedef, name: "__int64_t", file: !53, line: 46, baseType: !54)
!53 = !DIFile(filename: "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/i386/_types.h", directory: "")
!54 = !DIBasicType(name: "long long int", size: 64, encoding: DW_ATE_signed)
!55 = !DIDerivedType(tag: DW_TAG_member, name: "_write", scope: !17, file: !16, line: 140, baseType: !56, size: 64, offset: 640)
!56 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !57, size: 64)
!57 = !DISubroutineType(types: !58)
!58 = !{!11, !34, !59, !11}
!59 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !60, size: 64)
!60 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !44)
!61 = !DIDerivedType(tag: DW_TAG_member, name: "_ub", scope: !17, file: !16, line: 143, baseType: !28, size: 128, offset: 704)
!62 = !DIDerivedType(tag: DW_TAG_member, name: "_extra", scope: !17, file: !16, line: 144, baseType: !63, size: 64, offset: 832)
!63 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !64, size: 64)
!64 = !DICompositeType(tag: DW_TAG_structure_type, name: "__sFILEX", file: !16, line: 98, flags: DIFlagFwdDecl)
!65 = !DIDerivedType(tag: DW_TAG_member, name: "_ur", scope: !17, file: !16, line: 145, baseType: !11, size: 32, offset: 896)
!66 = !DIDerivedType(tag: DW_TAG_member, name: "_ubuf", scope: !17, file: !16, line: 148, baseType: !67, size: 24, offset: 928)
!67 = !DICompositeType(tag: DW_TAG_array_type, baseType: !21, size: 24, elements: !68)
!68 = !{!69}
!69 = !DISubrange(count: 3)
!70 = !DIDerivedType(tag: DW_TAG_member, name: "_nbuf", scope: !17, file: !16, line: 149, baseType: !71, size: 8, offset: 952)
!71 = !DICompositeType(tag: DW_TAG_array_type, baseType: !21, size: 8, elements: !72)
!72 = !{!73}
!73 = !DISubrange(count: 1)
!74 = !DIDerivedType(tag: DW_TAG_member, name: "_lb", scope: !17, file: !16, line: 152, baseType: !28, size: 128, offset: 960)
!75 = !DIDerivedType(tag: DW_TAG_member, name: "_blksize", scope: !17, file: !16, line: 155, baseType: !11, size: 32, offset: 1088)
!76 = !DIDerivedType(tag: DW_TAG_member, name: "_offset", scope: !17, file: !16, line: 156, baseType: !49, size: 64, offset: 1152)
!77 = !DILocation(line: 0, scope: !8)
!78 = !DILocalVariable(name: "buf", scope: !8, file: !1, line: 5, type: !79)
!79 = !DICompositeType(tag: DW_TAG_array_type, baseType: !44, size: 8200, elements: !80)
!80 = !{!81}
!81 = !DISubrange(count: 1025)
!82 = !DILocation(line: 5, column: 8, scope: !8)
!83 = !DILocation(line: 7, column: 9, scope: !8)
!84 = !DILocation(line: 7, column: 3, scope: !8)
!85 = !DILocation(line: 8, column: 3, scope: !8)
!86 = !DILocation(line: 9, column: 3, scope: !8)
!87 = !DILocalVariable(name: "x", scope: !8, file: !1, line: 6, type: !11)
!88 = !DILocation(line: 11, column: 3, scope: !8)
!89 = !DILocation(line: 12, column: 3, scope: !8)
!90 = !DILocation(line: 13, column: 3, scope: !8)
!91 = !DILocation(line: 14, column: 1, scope: !8)
