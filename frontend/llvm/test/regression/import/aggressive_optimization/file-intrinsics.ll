; ModuleID = 'file-intrinsics.c.pp.bc'
source_filename = "file-intrinsics.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

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
define i32 @main() local_unnamed_addr #0 !dbg !7 {
  %1 = alloca [1025 x i8], align 16
  %2 = alloca i32, align 4
  %3 = getelementptr inbounds [10 x i8], [10 x i8]* @.str, i64 0, i64 0, !dbg !11
  %4 = getelementptr inbounds [3 x i8], [3 x i8]* @.str.1, i64 0, i64 0, !dbg !11
  %5 = call %struct.__sFILE* @"\01_fopen"(i8* %3, i8* %4) #3, !dbg !11
  call void @llvm.dbg.value(metadata %struct.__sFILE* %5, i64 0, metadata !12, metadata !76), !dbg !77
  call void @llvm.dbg.declare(metadata [1025 x i8]* %1, metadata !78, metadata !76), !dbg !82
  %6 = getelementptr inbounds [1025 x i8], [1025 x i8]* %1, i64 0, i64 0, !dbg !83
  %7 = call i8* @fgets(i8* %6, i32 1024, %struct.__sFILE* %5) #3, !dbg !84
  %8 = call i32 @fgetc(%struct.__sFILE* %5) #3, !dbg !85
  %9 = getelementptr inbounds [12 x i8], [12 x i8]* @.str.2, i64 0, i64 0, !dbg !86
  %10 = call i32 @"\01_fputs"(i8* %9, %struct.__sFILE* %5) #3, !dbg !86
  call void @llvm.dbg.value(metadata i32* %2, i64 0, metadata !87, metadata !88), !dbg !89
  %11 = getelementptr inbounds [3 x i8], [3 x i8]* @.str.3, i64 0, i64 0, !dbg !90
  %12 = call i32 (%struct.__sFILE*, i8*, ...) @fscanf(%struct.__sFILE* %5, i8* %11, i32* nonnull %2) #3, !dbg !90
  %13 = call i32 @fflush(%struct.__sFILE* %5) #3, !dbg !91
  %14 = call i32 @fclose(%struct.__sFILE* %5) #3, !dbg !92
  ret i32 0, !dbg !93
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   [1025 x si8]* $1 = allocate [1025 x si8], 1, align 16
; CHECK:   si32* $2 = allocate si32, 1, align 4
; CHECK:   si8* %3 = ptrshift @.str, 10 * 0, 1 * 0
; CHECK:   si8* %4 = ptrshift @.str.1, 3 * 0, 1 * 0
; CHECK:   opaque* %5 = call @ar.libc.fopen(%3, %4)
; CHECK:   {0: ui8*, 8: si32, 12: si32, 16: si16, 18: si16, 24: {0: ui8*, 8: si32}, 40: si32, 48: si8*, 56: si32 (si8*)*, 64: si32 (si8*, si8*, si32)*, 72: si64 (si8*, si64, si32)*, 80: si32 (si8*, si8*, si32)*, 88: {...}, 104: opaque*, 112: si32, 116: [3 x ui8], 119: [1 x ui8], 120: {...}, 136: si32, 144: si64}* %6 = bitcast %5
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "file-intrinsics.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !8, isLocal: false, isDefinition: true, scopeLine: 3, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocation(line: 4, column: 13, scope: !7)
!12 = !DILocalVariable(name: "f", scope: !7, file: !1, line: 4, type: !13)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIDerivedType(tag: DW_TAG_typedef, name: "FILE", file: !15, line: 153, baseType: !16)
!15 = !DIFile(filename: "/usr/include/stdio.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!16 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__sFILE", file: !15, line: 122, size: 1216, elements: !17)
!17 = !{!18, !21, !22, !23, !25, !26, !31, !32, !34, !38, !44, !54, !60, !61, !64, !65, !69, !73, !74, !75}
!18 = !DIDerivedType(tag: DW_TAG_member, name: "_p", scope: !16, file: !15, line: 123, baseType: !19, size: 64)
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!20 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!21 = !DIDerivedType(tag: DW_TAG_member, name: "_r", scope: !16, file: !15, line: 124, baseType: !10, size: 32, offset: 64)
!22 = !DIDerivedType(tag: DW_TAG_member, name: "_w", scope: !16, file: !15, line: 125, baseType: !10, size: 32, offset: 96)
!23 = !DIDerivedType(tag: DW_TAG_member, name: "_flags", scope: !16, file: !15, line: 126, baseType: !24, size: 16, offset: 128)
!24 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!25 = !DIDerivedType(tag: DW_TAG_member, name: "_file", scope: !16, file: !15, line: 127, baseType: !24, size: 16, offset: 144)
!26 = !DIDerivedType(tag: DW_TAG_member, name: "_bf", scope: !16, file: !15, line: 128, baseType: !27, size: 128, offset: 192)
!27 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__sbuf", file: !15, line: 88, size: 128, elements: !28)
!28 = !{!29, !30}
!29 = !DIDerivedType(tag: DW_TAG_member, name: "_base", scope: !27, file: !15, line: 89, baseType: !19, size: 64)
!30 = !DIDerivedType(tag: DW_TAG_member, name: "_size", scope: !27, file: !15, line: 90, baseType: !10, size: 32, offset: 64)
!31 = !DIDerivedType(tag: DW_TAG_member, name: "_lbfsize", scope: !16, file: !15, line: 129, baseType: !10, size: 32, offset: 320)
!32 = !DIDerivedType(tag: DW_TAG_member, name: "_cookie", scope: !16, file: !15, line: 132, baseType: !33, size: 64, offset: 384)
!33 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!34 = !DIDerivedType(tag: DW_TAG_member, name: "_close", scope: !16, file: !15, line: 133, baseType: !35, size: 64, offset: 448)
!35 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !36, size: 64)
!36 = !DISubroutineType(types: !37)
!37 = !{!10, !33}
!38 = !DIDerivedType(tag: DW_TAG_member, name: "_read", scope: !16, file: !15, line: 134, baseType: !39, size: 64, offset: 512)
!39 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !40, size: 64)
!40 = !DISubroutineType(types: !41)
!41 = !{!10, !33, !42, !10}
!42 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !43, size: 64)
!43 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!44 = !DIDerivedType(tag: DW_TAG_member, name: "_seek", scope: !16, file: !15, line: 135, baseType: !45, size: 64, offset: 576)
!45 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !46, size: 64)
!46 = !DISubroutineType(types: !47)
!47 = !{!48, !33, !48, !10}
!48 = !DIDerivedType(tag: DW_TAG_typedef, name: "fpos_t", file: !15, line: 77, baseType: !49)
!49 = !DIDerivedType(tag: DW_TAG_typedef, name: "__darwin_off_t", file: !50, line: 71, baseType: !51)
!50 = !DIFile(filename: "/usr/include/sys/_types.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!51 = !DIDerivedType(tag: DW_TAG_typedef, name: "__int64_t", file: !52, line: 46, baseType: !53)
!52 = !DIFile(filename: "/usr/include/i386/_types.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!53 = !DIBasicType(name: "long long int", size: 64, encoding: DW_ATE_signed)
!54 = !DIDerivedType(tag: DW_TAG_member, name: "_write", scope: !16, file: !15, line: 136, baseType: !55, size: 64, offset: 640)
!55 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !56, size: 64)
!56 = !DISubroutineType(types: !57)
!57 = !{!10, !33, !58, !10}
!58 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !59, size: 64)
!59 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !43)
!60 = !DIDerivedType(tag: DW_TAG_member, name: "_ub", scope: !16, file: !15, line: 139, baseType: !27, size: 128, offset: 704)
!61 = !DIDerivedType(tag: DW_TAG_member, name: "_extra", scope: !16, file: !15, line: 140, baseType: !62, size: 64, offset: 832)
!62 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !63, size: 64)
!63 = !DICompositeType(tag: DW_TAG_structure_type, name: "__sFILEX", file: !15, line: 94, flags: DIFlagFwdDecl)
!64 = !DIDerivedType(tag: DW_TAG_member, name: "_ur", scope: !16, file: !15, line: 141, baseType: !10, size: 32, offset: 896)
!65 = !DIDerivedType(tag: DW_TAG_member, name: "_ubuf", scope: !16, file: !15, line: 144, baseType: !66, size: 24, offset: 928)
!66 = !DICompositeType(tag: DW_TAG_array_type, baseType: !20, size: 24, elements: !67)
!67 = !{!68}
!68 = !DISubrange(count: 3)
!69 = !DIDerivedType(tag: DW_TAG_member, name: "_nbuf", scope: !16, file: !15, line: 145, baseType: !70, size: 8, offset: 952)
!70 = !DICompositeType(tag: DW_TAG_array_type, baseType: !20, size: 8, elements: !71)
!71 = !{!72}
!72 = !DISubrange(count: 1)
!73 = !DIDerivedType(tag: DW_TAG_member, name: "_lb", scope: !16, file: !15, line: 148, baseType: !27, size: 128, offset: 960)
!74 = !DIDerivedType(tag: DW_TAG_member, name: "_blksize", scope: !16, file: !15, line: 151, baseType: !10, size: 32, offset: 1088)
!75 = !DIDerivedType(tag: DW_TAG_member, name: "_offset", scope: !16, file: !15, line: 152, baseType: !48, size: 64, offset: 1152)
!76 = !DIExpression()
!77 = !DILocation(line: 4, column: 9, scope: !7)
!78 = !DILocalVariable(name: "buf", scope: !7, file: !1, line: 5, type: !79)
!79 = !DICompositeType(tag: DW_TAG_array_type, baseType: !43, size: 8200, elements: !80)
!80 = !{!81}
!81 = !DISubrange(count: 1025)
!82 = !DILocation(line: 5, column: 8, scope: !7)
!83 = !DILocation(line: 7, column: 9, scope: !7)
!84 = !DILocation(line: 7, column: 3, scope: !7)
!85 = !DILocation(line: 8, column: 3, scope: !7)
!86 = !DILocation(line: 9, column: 3, scope: !7)
!87 = !DILocalVariable(name: "x", scope: !7, file: !1, line: 6, type: !10)
!88 = !DIExpression(DW_OP_deref)
!89 = !DILocation(line: 6, column: 7, scope: !7)
!90 = !DILocation(line: 11, column: 3, scope: !7)
!91 = !DILocation(line: 12, column: 3, scope: !7)
!92 = !DILocation(line: 13, column: 3, scope: !7)
!93 = !DILocation(line: 14, column: 1, scope: !7)
