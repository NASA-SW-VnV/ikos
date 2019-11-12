; ModuleID = 'mem-intrinsics.pp.bc'
source_filename = "mem-intrinsics.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #2
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memmove.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i1) #2
; CHECK: declare void @ar.memmove(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #2
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @cst() #0 !dbg !11 {
  %1 = alloca i32, align 4
  call void @llvm.dbg.declare(metadata i32* %1, metadata !14, metadata !DIExpression()), !dbg !15
  store i32 10, i32* %1, align 4, !dbg !15
  %2 = load i32, i32* %1, align 4, !dbg !16
  ret i32 %2, !dbg !17
}
; CHECK: define si32 @cst() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   store $1, 10, align 4
; CHECK:   si32 %2 = load $1, align 4
; CHECK:   return %2
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !18 {
  %1 = alloca i32*, align 8
  %2 = alloca i32*, align 8
  %3 = alloca i32*, align 8
  call void @llvm.dbg.declare(metadata i32** %1, metadata !19, metadata !DIExpression()), !dbg !20
  call void @llvm.dbg.declare(metadata i32** %2, metadata !21, metadata !DIExpression()), !dbg !22
  call void @llvm.dbg.declare(metadata i32** %3, metadata !23, metadata !DIExpression()), !dbg !24
  %4 = load i32*, i32** %1, align 8, !dbg !25
  %5 = bitcast i32* %4 to i8*, !dbg !26
  %6 = load i32*, i32** %2, align 8, !dbg !27
  %7 = bitcast i32* %6 to i8*, !dbg !26
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %5, i8* align 4 %7, i64 10, i1 false), !dbg !26
  %8 = bitcast i8* %5 to i32*, !dbg !28
  store i32* %8, i32** %3, align 8, !dbg !29
  %9 = load i32*, i32** %1, align 8, !dbg !30
  %10 = bitcast i32* %9 to i8*, !dbg !31
  %11 = load i32*, i32** %2, align 8, !dbg !32
  %12 = bitcast i32* %11 to i8*, !dbg !31
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 4 %10, i8* align 4 %12, i64 50, i1 false), !dbg !31
  %13 = bitcast i8* %10 to i32*, !dbg !33
  store i32* %13, i32** %3, align 8, !dbg !34
  %14 = load i32*, i32** %1, align 8, !dbg !35
  %15 = bitcast i32* %14 to i8*, !dbg !36
  call void @llvm.memset.p0i8.i64(i8* align 4 %15, i8 1, i64 50, i1 false), !dbg !36
  %16 = bitcast i8* %15 to i32*, !dbg !37
  store i32* %16, i32** %3, align 8, !dbg !38
  ret i32 0, !dbg !39
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32** $1 = allocate si32*, 1, align 8
; CHECK:   si32** $2 = allocate si32*, 1, align 8
; CHECK:   si32** $3 = allocate si32*, 1, align 8
; CHECK:   si32* %4 = load $1, align 8
; CHECK:   si8* %5 = bitcast %4
; CHECK:   si32* %6 = load $2, align 8
; CHECK:   si8* %7 = bitcast %6
; CHECK:   call @ar.memcpy(%5, %7, 10, 4, 4, 0)
; CHECK:   si32* %8 = bitcast %5
; CHECK:   store $3, %8, align 8
; CHECK:   si32* %9 = load $1, align 8
; CHECK:   si8* %10 = bitcast %9
; CHECK:   si32* %11 = load $2, align 8
; CHECK:   si8* %12 = bitcast %11
; CHECK:   call @ar.memmove(%10, %12, 50, 4, 4, 0)
; CHECK:   si32* %13 = bitcast %10
; CHECK:   store $3, %13, align 8
; CHECK:   si32* %14 = load $1, align 8
; CHECK:   si8* %15 = bitcast %14
; CHECK:   call @ar.memset(%15, 1, 50, 4, 0)
; CHECK:   si32* %16 = bitcast %15
; CHECK:   store $3, %16, align 8
; CHECK:   return 0
; CHECK: }
; CHECK: }

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!6, !7, !8, !9}
!llvm.ident = !{!10}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "mem-intrinsics.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{!4}
!4 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!5 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{i32 7, !"PIC Level", i32 2}
!10 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!11 = distinct !DISubprogram(name: "cst", scope: !1, file: !1, line: 3, type: !12, scopeLine: 3, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!12 = !DISubroutineType(types: !13)
!13 = !{!5}
!14 = !DILocalVariable(name: "G", scope: !11, file: !1, line: 4, type: !5)
!15 = !DILocation(line: 4, column: 7, scope: !11)
!16 = !DILocation(line: 5, column: 10, scope: !11)
!17 = !DILocation(line: 5, column: 3, scope: !11)
!18 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 8, type: !12, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!19 = !DILocalVariable(name: "p", scope: !18, file: !1, line: 9, type: !4)
!20 = !DILocation(line: 9, column: 8, scope: !18)
!21 = !DILocalVariable(name: "q", scope: !18, file: !1, line: 9, type: !4)
!22 = !DILocation(line: 9, column: 12, scope: !18)
!23 = !DILocalVariable(name: "r", scope: !18, file: !1, line: 9, type: !4)
!24 = !DILocation(line: 9, column: 16, scope: !18)
!25 = !DILocation(line: 10, column: 20, scope: !18)
!26 = !DILocation(line: 10, column: 13, scope: !18)
!27 = !DILocation(line: 10, column: 23, scope: !18)
!28 = !DILocation(line: 10, column: 7, scope: !18)
!29 = !DILocation(line: 10, column: 5, scope: !18)
!30 = !DILocation(line: 11, column: 21, scope: !18)
!31 = !DILocation(line: 11, column: 13, scope: !18)
!32 = !DILocation(line: 11, column: 24, scope: !18)
!33 = !DILocation(line: 11, column: 7, scope: !18)
!34 = !DILocation(line: 11, column: 5, scope: !18)
!35 = !DILocation(line: 12, column: 20, scope: !18)
!36 = !DILocation(line: 12, column: 13, scope: !18)
!37 = !DILocation(line: 12, column: 7, scope: !18)
!38 = !DILocation(line: 12, column: 5, scope: !18)
!39 = !DILocation(line: 13, column: 1, scope: !18)
