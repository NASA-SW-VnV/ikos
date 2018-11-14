; ModuleID = 'vla.c.pp.bc'
source_filename = "vla.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
; CHECK: define [3 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [37, 100, 0], align 1
; CHECK: }
; CHECK: }

declare i32 @scanf(i8*, ...) local_unnamed_addr #1
; CHECK: declare si32 @ar.libc.scanf(si8*, ...)

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @foo(i32) unnamed_addr #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !12, metadata !DIExpression()), !dbg !13
  call void @llvm.dbg.value(metadata i32 0, metadata !14, metadata !DIExpression()), !dbg !15
  ret void, !dbg !16
}
; CHECK: define void @foo(si32 %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !17 {
  %3 = alloca i32, align 4
  call void @llvm.dbg.value(metadata i32 %0, metadata !23, metadata !DIExpression()), !dbg !24
  call void @llvm.dbg.value(metadata i8** %1, metadata !25, metadata !DIExpression()), !dbg !26
  call void @llvm.dbg.value(metadata i32* %3, metadata !27, metadata !DIExpression()), !dbg !28
  %4 = getelementptr inbounds [3 x i8], [3 x i8]* @.str, i64 0, i64 0, !dbg !29
  %5 = call i32 (i8*, ...) @scanf(i8* %4, i32* nonnull %3) #3, !dbg !29
  %6 = load i32, i32* %3, align 4, !dbg !30
  call void @llvm.dbg.value(metadata i32 %6, metadata !27, metadata !DIExpression()), !dbg !28
  call fastcc void @foo(i32 %6), !dbg !31
  ret i32 0, !dbg !32
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si8* %4 = ptrshift @.str, 3 * 0, 1 * 0
; CHECK:   si32 %5 = call @ar.libc.scanf(%4, $3)
; CHECK:   si32 %6 = load $3, align 4
; CHECK:   call @foo(%6)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 6.0.1 (tags/RELEASE_601/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "vla.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 6.0.1 (tags/RELEASE_601/final)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 2, type: !9, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "n", arg: 1, scope: !8, file: !1, line: 2, type: !11)
!13 = !DILocation(line: 2, column: 14, scope: !8)
!14 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 3, type: !11)
!15 = !DILocation(line: 3, column: 13, scope: !8)
!16 = !DILocation(line: 8, column: 1, scope: !8)
!17 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !18, isLocal: false, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!18 = !DISubroutineType(types: !19)
!19 = !{!11, !11, !20}
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !21, size: 64)
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !22, size: 64)
!22 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!23 = !DILocalVariable(name: "argc", arg: 1, scope: !17, file: !1, line: 10, type: !11)
!24 = !DILocation(line: 10, column: 14, scope: !17)
!25 = !DILocalVariable(name: "argv", arg: 2, scope: !17, file: !1, line: 10, type: !20)
!26 = !DILocation(line: 10, column: 27, scope: !17)
!27 = !DILocalVariable(name: "v", scope: !17, file: !1, line: 11, type: !11)
!28 = !DILocation(line: 11, column: 7, scope: !17)
!29 = !DILocation(line: 12, column: 3, scope: !17)
!30 = !DILocation(line: 13, column: 7, scope: !17)
!31 = !DILocation(line: 13, column: 3, scope: !17)
!32 = !DILocation(line: 14, column: 3, scope: !17)
