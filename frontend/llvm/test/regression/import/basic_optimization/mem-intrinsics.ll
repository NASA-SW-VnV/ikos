; ModuleID = 'mem-intrinsics.pp.bc'
source_filename = "mem-intrinsics.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #1
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memmove.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i1 immarg) #1
; CHECK: declare void @ar.memmove(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #1
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @cst() #0 !dbg !11 {
  call void @llvm.dbg.value(metadata i32 10, metadata !14, metadata !DIExpression()), !dbg !15
  ret i32 10, !dbg !16
}
; CHECK: define si32 @cst() {
; CHECK: #1 !entry !exit {
; CHECK:   return 10
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !17 {
  %1 = bitcast i32* undef to i8*, !dbg !18
  %2 = bitcast i32* undef to i8*, !dbg !18
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 4 %2, i64 10, i1 false), !dbg !18
  call void @llvm.dbg.value(metadata i8* %1, metadata !19, metadata !DIExpression()), !dbg !20
  %3 = bitcast i32* undef to i8*, !dbg !21
  %4 = bitcast i32* undef to i8*, !dbg !21
  call void @llvm.memmove.p0i8.p0i8.i64(i8* align 4 %3, i8* align 4 %4, i64 50, i1 false), !dbg !21
  call void @llvm.dbg.value(metadata i8* %3, metadata !19, metadata !DIExpression()), !dbg !20
  %5 = bitcast i32* undef to i8*, !dbg !22
  call void @llvm.memset.p0i8.i64(i8* align 4 %5, i8 1, i64 50, i1 false), !dbg !22
  call void @llvm.dbg.value(metadata i8* %5, metadata !19, metadata !DIExpression()), !dbg !20
  ret i32 0, !dbg !23
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast undef
; CHECK:   si8* %2 = bitcast undef
; CHECK:   call @ar.memcpy(%1, %2, 10, 4, 4, 0)
; CHECK:   si8* %3 = bitcast undef
; CHECK:   si8* %4 = bitcast undef
; CHECK:   call @ar.memmove(%3, %4, 50, 4, 4, 0)
; CHECK:   si8* %5 = bitcast undef
; CHECK:   call @ar.memset(%5, 1, 50, 4, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!6, !7, !8, !9}
!llvm.ident = !{!10}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "mem-intrinsics.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
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
!15 = !DILocation(line: 0, scope: !11)
!16 = !DILocation(line: 5, column: 3, scope: !11)
!17 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 8, type: !12, scopeLine: 8, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!18 = !DILocation(line: 10, column: 13, scope: !17)
!19 = !DILocalVariable(name: "r", scope: !17, file: !1, line: 9, type: !4)
!20 = !DILocation(line: 0, scope: !17)
!21 = !DILocation(line: 11, column: 13, scope: !17)
!22 = !DILocation(line: 12, column: 13, scope: !17)
!23 = !DILocation(line: 13, column: 1, scope: !17)
