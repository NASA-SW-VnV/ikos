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

declare i32 @fclose(%struct.__sFILE*) #2
; CHECK: declare si32 @ar.libc.fclose(opaque*)

declare i32 @fflush(%struct.__sFILE*) #2
; CHECK: declare si32 @ar.libc.fflush(opaque*)

declare i32 @fgetc(%struct.__sFILE*) #2
; CHECK: declare si32 @ar.libc.fgetc(opaque*)

declare i8* @fgets(i8*, i32, %struct.__sFILE*) #2
; CHECK: declare si8* @ar.libc.fgets(si8*, si32, opaque*)

declare %struct.__sFILE* @"\01_fopen"(i8*, i8*) #2
; CHECK: declare opaque* @ar.libc.fopen(si8*, si8*)

declare i32 @fprintf(%struct.__sFILE*, i8*, ...) #2
; CHECK: declare si32 @ar.libc.fprintf(opaque*, si8*, ...)

declare i32 @"\01_fputs"(i8*, %struct.__sFILE*) #2
; CHECK: declare si32 @ar.libc.fputs(si8*, opaque*)

declare i32 @fscanf(%struct.__sFILE*, i8*, ...) #2
; CHECK: declare si32 @ar.libc.fscanf(opaque*, si8*, ...)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !7 {
  %1 = alloca %struct.__sFILE*, align 8
  %2 = alloca [1025 x i8], align 16
  %3 = alloca i32, align 4
  call void @llvm.dbg.declare(metadata %struct.__sFILE** %1, metadata !11, metadata !75), !dbg !76
  %4 = getelementptr inbounds [10 x i8], [10 x i8]* @.str, i32 0, i32 0, !dbg !77
  %5 = getelementptr inbounds [3 x i8], [3 x i8]* @.str.1, i32 0, i32 0, !dbg !77
  %6 = call %struct.__sFILE* @"\01_fopen"(i8* %4, i8* %5), !dbg !77
  store %struct.__sFILE* %6, %struct.__sFILE** %1, align 8, !dbg !76
  call void @llvm.dbg.declare(metadata [1025 x i8]* %2, metadata !78, metadata !75), !dbg !82
  call void @llvm.dbg.declare(metadata i32* %3, metadata !83, metadata !75), !dbg !84
  %7 = getelementptr inbounds [1025 x i8], [1025 x i8]* %2, i32 0, i32 0, !dbg !85
  %8 = load %struct.__sFILE*, %struct.__sFILE** %1, align 8, !dbg !86
  %9 = call i8* @fgets(i8* %7, i32 1024, %struct.__sFILE* %8), !dbg !87
  %10 = load %struct.__sFILE*, %struct.__sFILE** %1, align 8, !dbg !88
  %11 = call i32 @fgetc(%struct.__sFILE* %10), !dbg !89
  %12 = load %struct.__sFILE*, %struct.__sFILE** %1, align 8, !dbg !90
  %13 = getelementptr inbounds [12 x i8], [12 x i8]* @.str.2, i32 0, i32 0, !dbg !91
  %14 = call i32 @"\01_fputs"(i8* %13, %struct.__sFILE* %12), !dbg !91
  %15 = load %struct.__sFILE*, %struct.__sFILE** %1, align 8, !dbg !92
  %16 = getelementptr inbounds [3 x i8], [3 x i8]* @.str.3, i32 0, i32 0, !dbg !93
  %17 = call i32 (%struct.__sFILE*, i8*, ...) @fprintf(%struct.__sFILE* %15, i8* %16, i32 1), !dbg !93
  %18 = load %struct.__sFILE*, %struct.__sFILE** %1, align 8, !dbg !94
  %19 = getelementptr inbounds [3 x i8], [3 x i8]* @.str.3, i32 0, i32 0, !dbg !95
  %20 = call i32 (%struct.__sFILE*, i8*, ...) @fscanf(%struct.__sFILE* %18, i8* %19, i32* %3), !dbg !95
  %21 = load %struct.__sFILE*, %struct.__sFILE** %1, align 8, !dbg !96
  %22 = call i32 @fflush(%struct.__sFILE* %21), !dbg !97
  %23 = load %struct.__sFILE*, %struct.__sFILE** %1, align 8, !dbg !98
  %24 = call i32 @fclose(%struct.__sFILE* %23), !dbg !99
  ret i32 0, !dbg !100
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: ui8*, 8: si32, 12: si32, 16: si16, 18: si16, 24: {0: ui8*, 8: si32}, 40: si32, 48: si8*, 56: si32 (si8*)*, 64: si32 (si8*, si8*, si32)*, 72: si64 (si8*, si64, si32)*, 80: si32 (si8*, si8*, si32)*, 88: {...}, 104: opaque*, 112: si32, 116: [3 x ui8], 119: [1 x ui8], 120: {...}, 136: si32, 144: si64}** $1 = allocate {0: ui8*, 8: si32, 12: si32, 16: si16, 18: si16, 24: {0: ui8*, 8: si32}, 40: si32, 48: si8*, 56: si32 (si8*)*, 64: si32 (si8*, si8*, si32)*, 72: si64 (si8*, si64, si32)*, 80: si32 (si8*, si8*, si32)*, 88: {...}, 104: opaque*, 112: si32, 116: [3 x ui8], 119: [1 x ui8], 120: {...}, 136: si32, 144: si64}*, 1, align 8
; CHECK:   [1025 x si8]* $2 = allocate [1025 x si8], 1, align 16
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si8* %4 = ptrshift @.str, 10 * 0, 1 * 0
; CHECK:   si8* %5 = ptrshift @.str.1, 3 * 0, 1 * 0
; CHECK:   opaque* %6 = call @ar.libc.fopen(%4, %5)
; CHECK:   {0: ui8*, 8: si32, 12: si32, 16: si16, 18: si16, 24: {0: ui8*, 8: si32}, 40: si32, 48: si8*, 56: si32 (si8*)*, 64: si32 (si8*, si8*, si32)*, 72: si64 (si8*, si64, si32)*, 80: si32 (si8*, si8*, si32)*, 88: {...}, 104: opaque*, 112: si32, 116: [3 x ui8], 119: [1 x ui8], 120: {...}, 136: si32, 144: si64}* %7 = bitcast %6
; CHECK:   store $1, %7, align 8
; CHECK:   si8* %8 = ptrshift $2, 1025 * 0, 1 * 0
; CHECK:   opaque** %9 = bitcast $1
; CHECK:   opaque* %10 = load %9, align 8
; CHECK:   si8* %11 = call @ar.libc.fgets(%8, 1024, %10)
; CHECK:   opaque** %12 = bitcast $1
; CHECK:   opaque* %13 = load %12, align 8
; CHECK:   si32 %14 = call @ar.libc.fgetc(%13)
; CHECK:   opaque** %15 = bitcast $1
; CHECK:   opaque* %16 = load %15, align 8
; CHECK:   si8* %17 = ptrshift @.str.2, 12 * 0, 1 * 0
; CHECK:   si32 %18 = call @ar.libc.fputs(%17, %16)
; CHECK:   opaque** %19 = bitcast $1
; CHECK:   opaque* %20 = load %19, align 8
; CHECK:   si8* %21 = ptrshift @.str.3, 3 * 0, 1 * 0
; CHECK:   si32 %22 = call @ar.libc.fprintf(%20, %21, 1)
; CHECK:   opaque** %23 = bitcast $1
; CHECK:   opaque* %24 = load %23, align 8
; CHECK:   si8* %25 = ptrshift @.str.3, 3 * 0, 1 * 0
; CHECK:   si32 %26 = call @ar.libc.fscanf(%24, %25, $3)
; CHECK:   opaque** %27 = bitcast $1
; CHECK:   opaque* %28 = load %27, align 8
; CHECK:   si32 %29 = call @ar.libc.fflush(%28)
; CHECK:   opaque** %30 = bitcast $1
; CHECK:   opaque* %31 = load %30, align 8
; CHECK:   si32 %32 = call @ar.libc.fclose(%31)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "file-intrinsics.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !8, isLocal: false, isDefinition: true, scopeLine: 3, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "f", scope: !7, file: !1, line: 4, type: !12)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIDerivedType(tag: DW_TAG_typedef, name: "FILE", file: !14, line: 153, baseType: !15)
!14 = !DIFile(filename: "/usr/include/stdio.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!15 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__sFILE", file: !14, line: 122, size: 1216, elements: !16)
!16 = !{!17, !20, !21, !22, !24, !25, !30, !31, !33, !37, !43, !53, !59, !60, !63, !64, !68, !72, !73, !74}
!17 = !DIDerivedType(tag: DW_TAG_member, name: "_p", scope: !15, file: !14, line: 123, baseType: !18, size: 64)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!19 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!20 = !DIDerivedType(tag: DW_TAG_member, name: "_r", scope: !15, file: !14, line: 124, baseType: !10, size: 32, offset: 64)
!21 = !DIDerivedType(tag: DW_TAG_member, name: "_w", scope: !15, file: !14, line: 125, baseType: !10, size: 32, offset: 96)
!22 = !DIDerivedType(tag: DW_TAG_member, name: "_flags", scope: !15, file: !14, line: 126, baseType: !23, size: 16, offset: 128)
!23 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!24 = !DIDerivedType(tag: DW_TAG_member, name: "_file", scope: !15, file: !14, line: 127, baseType: !23, size: 16, offset: 144)
!25 = !DIDerivedType(tag: DW_TAG_member, name: "_bf", scope: !15, file: !14, line: 128, baseType: !26, size: 128, offset: 192)
!26 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "__sbuf", file: !14, line: 88, size: 128, elements: !27)
!27 = !{!28, !29}
!28 = !DIDerivedType(tag: DW_TAG_member, name: "_base", scope: !26, file: !14, line: 89, baseType: !18, size: 64)
!29 = !DIDerivedType(tag: DW_TAG_member, name: "_size", scope: !26, file: !14, line: 90, baseType: !10, size: 32, offset: 64)
!30 = !DIDerivedType(tag: DW_TAG_member, name: "_lbfsize", scope: !15, file: !14, line: 129, baseType: !10, size: 32, offset: 320)
!31 = !DIDerivedType(tag: DW_TAG_member, name: "_cookie", scope: !15, file: !14, line: 132, baseType: !32, size: 64, offset: 384)
!32 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "_close", scope: !15, file: !14, line: 133, baseType: !34, size: 64, offset: 448)
!34 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !35, size: 64)
!35 = !DISubroutineType(types: !36)
!36 = !{!10, !32}
!37 = !DIDerivedType(tag: DW_TAG_member, name: "_read", scope: !15, file: !14, line: 134, baseType: !38, size: 64, offset: 512)
!38 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !39, size: 64)
!39 = !DISubroutineType(types: !40)
!40 = !{!10, !32, !41, !10}
!41 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !42, size: 64)
!42 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!43 = !DIDerivedType(tag: DW_TAG_member, name: "_seek", scope: !15, file: !14, line: 135, baseType: !44, size: 64, offset: 576)
!44 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !45, size: 64)
!45 = !DISubroutineType(types: !46)
!46 = !{!47, !32, !47, !10}
!47 = !DIDerivedType(tag: DW_TAG_typedef, name: "fpos_t", file: !14, line: 77, baseType: !48)
!48 = !DIDerivedType(tag: DW_TAG_typedef, name: "__darwin_off_t", file: !49, line: 71, baseType: !50)
!49 = !DIFile(filename: "/usr/include/sys/_types.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!50 = !DIDerivedType(tag: DW_TAG_typedef, name: "__int64_t", file: !51, line: 46, baseType: !52)
!51 = !DIFile(filename: "/usr/include/i386/_types.h", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!52 = !DIBasicType(name: "long long int", size: 64, encoding: DW_ATE_signed)
!53 = !DIDerivedType(tag: DW_TAG_member, name: "_write", scope: !15, file: !14, line: 136, baseType: !54, size: 64, offset: 640)
!54 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !55, size: 64)
!55 = !DISubroutineType(types: !56)
!56 = !{!10, !32, !57, !10}
!57 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !58, size: 64)
!58 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !42)
!59 = !DIDerivedType(tag: DW_TAG_member, name: "_ub", scope: !15, file: !14, line: 139, baseType: !26, size: 128, offset: 704)
!60 = !DIDerivedType(tag: DW_TAG_member, name: "_extra", scope: !15, file: !14, line: 140, baseType: !61, size: 64, offset: 832)
!61 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !62, size: 64)
!62 = !DICompositeType(tag: DW_TAG_structure_type, name: "__sFILEX", file: !14, line: 94, flags: DIFlagFwdDecl)
!63 = !DIDerivedType(tag: DW_TAG_member, name: "_ur", scope: !15, file: !14, line: 141, baseType: !10, size: 32, offset: 896)
!64 = !DIDerivedType(tag: DW_TAG_member, name: "_ubuf", scope: !15, file: !14, line: 144, baseType: !65, size: 24, offset: 928)
!65 = !DICompositeType(tag: DW_TAG_array_type, baseType: !19, size: 24, elements: !66)
!66 = !{!67}
!67 = !DISubrange(count: 3)
!68 = !DIDerivedType(tag: DW_TAG_member, name: "_nbuf", scope: !15, file: !14, line: 145, baseType: !69, size: 8, offset: 952)
!69 = !DICompositeType(tag: DW_TAG_array_type, baseType: !19, size: 8, elements: !70)
!70 = !{!71}
!71 = !DISubrange(count: 1)
!72 = !DIDerivedType(tag: DW_TAG_member, name: "_lb", scope: !15, file: !14, line: 148, baseType: !26, size: 128, offset: 960)
!73 = !DIDerivedType(tag: DW_TAG_member, name: "_blksize", scope: !15, file: !14, line: 151, baseType: !10, size: 32, offset: 1088)
!74 = !DIDerivedType(tag: DW_TAG_member, name: "_offset", scope: !15, file: !14, line: 152, baseType: !47, size: 64, offset: 1152)
!75 = !DIExpression()
!76 = !DILocation(line: 4, column: 9, scope: !7)
!77 = !DILocation(line: 4, column: 13, scope: !7)
!78 = !DILocalVariable(name: "buf", scope: !7, file: !1, line: 5, type: !79)
!79 = !DICompositeType(tag: DW_TAG_array_type, baseType: !42, size: 8200, elements: !80)
!80 = !{!81}
!81 = !DISubrange(count: 1025)
!82 = !DILocation(line: 5, column: 8, scope: !7)
!83 = !DILocalVariable(name: "x", scope: !7, file: !1, line: 6, type: !10)
!84 = !DILocation(line: 6, column: 7, scope: !7)
!85 = !DILocation(line: 7, column: 9, scope: !7)
!86 = !DILocation(line: 7, column: 20, scope: !7)
!87 = !DILocation(line: 7, column: 3, scope: !7)
!88 = !DILocation(line: 8, column: 9, scope: !7)
!89 = !DILocation(line: 8, column: 3, scope: !7)
!90 = !DILocation(line: 9, column: 24, scope: !7)
!91 = !DILocation(line: 9, column: 3, scope: !7)
!92 = !DILocation(line: 10, column: 11, scope: !7)
!93 = !DILocation(line: 10, column: 3, scope: !7)
!94 = !DILocation(line: 11, column: 10, scope: !7)
!95 = !DILocation(line: 11, column: 3, scope: !7)
!96 = !DILocation(line: 12, column: 10, scope: !7)
!97 = !DILocation(line: 12, column: 3, scope: !7)
!98 = !DILocation(line: 13, column: 10, scope: !7)
!99 = !DILocation(line: 13, column: 3, scope: !7)
!100 = !DILocation(line: 14, column: 1, scope: !7)
