; ModuleID = 'mem-intrinsics.c.pp.bc'
source_filename = "mem-intrinsics.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #1
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memmove.p0i8.p0i8.i64(i8* nocapture, i8* nocapture readonly, i64, i32, i1) #1
; CHECK: declare void @ar.memmove(si8*, si8*, ui64, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #1
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @cst() #0 !dbg !10 {
  call void @llvm.dbg.value(metadata i32 10, i64 0, metadata !13, metadata !14), !dbg !15
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
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %1, i8* %2, i64 10, i32 4, i1 false), !dbg !18
  %3 = bitcast i32* undef to i8*, !dbg !19
  %4 = bitcast i32* undef to i8*, !dbg !19
  call void @llvm.memmove.p0i8.p0i8.i64(i8* %3, i8* %4, i64 50, i32 4, i1 false), !dbg !19
  %5 = bitcast i32* undef to i8*, !dbg !20
  call void @llvm.memset.p0i8.i64(i8* %5, i8 1, i64 50, i32 4, i1 false), !dbg !20
  ret i32 0, !dbg !21
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast undef
; CHECK:   si8* %2 = bitcast undef
; CHECK:   call @ar.memcpy(%1, %2, 10, 4, 0)
; CHECK:   si8* %3 = bitcast undef
; CHECK:   si8* %4 = bitcast undef
; CHECK:   call @ar.memmove(%3, %4, 50, 4, 0)
; CHECK:   si8* %5 = bitcast undef
; CHECK:   call @ar.memset(%5, 1, 50, 4, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #2

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!6, !7, !8}
!llvm.ident = !{!9}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, retainedTypes: !3)
!1 = !DIFile(filename: "mem-intrinsics.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
!2 = !{}
!3 = !{!4}
!4 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64)
!5 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!6 = !{i32 2, !"Dwarf Version", i32 4}
!7 = !{i32 2, !"Debug Info Version", i32 3}
!8 = !{i32 1, !"PIC Level", i32 2}
!9 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!10 = distinct !DISubprogram(name: "cst", scope: !1, file: !1, line: 3, type: !11, isLocal: false, isDefinition: true, scopeLine: 3, isOptimized: false, unit: !0, variables: !2)
!11 = !DISubroutineType(types: !12)
!12 = !{!5}
!13 = !DILocalVariable(name: "G", scope: !10, file: !1, line: 4, type: !5)
!14 = !DIExpression()
!15 = !DILocation(line: 4, column: 7, scope: !10)
!16 = !DILocation(line: 5, column: 3, scope: !10)
!17 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 8, type: !11, isLocal: false, isDefinition: true, scopeLine: 8, isOptimized: false, unit: !0, variables: !2)
!18 = !DILocation(line: 10, column: 13, scope: !17)
!19 = !DILocation(line: 11, column: 13, scope: !17)
!20 = !DILocation(line: 12, column: 13, scope: !17)
!21 = !DILocation(line: 13, column: 1, scope: !17)
