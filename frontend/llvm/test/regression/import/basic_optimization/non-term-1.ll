; ModuleID = 'non-term-1.pp.bc'
source_filename = "non-term-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noreturn
declare void @exit(i32) #1
; CHECK: declare void @ar.libc.exit(si32)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !8 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  call void @llvm.dbg.value(metadata i8** %1, metadata !17, metadata !DIExpression()), !dbg !16
  br label %3, !dbg !18

3:                                                ; preds = %2, %6
  %.0 = phi i32 [ undef, %2 ], [ %7, %6 ]
  call void @llvm.dbg.value(metadata i32 %.0, metadata !19, metadata !DIExpression()), !dbg !16
  %4 = icmp eq i32 %.0, 0, !dbg !20
  br i1 %4, label %5, label %6, !dbg !23

5:                                                ; preds = %3
  call void @exit(i32 1) #3, !dbg !24
  unreachable, !dbg !24

6:                                                ; preds = %3
  %7 = add nsw i32 %.0, 1, !dbg !25
  call void @llvm.dbg.value(metadata i32 %7, metadata !19, metadata !DIExpression()), !dbg !16
  br label %3, !dbg !18, !llvm.loop !26
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32 %.0 = undef
; CHECK: }
; CHECK: #2 predecessors={#1, #4} successors={#3, #4} {
; CHECK: }
; CHECK: #3 !exit predecessors={#2} {
; CHECK:   %.0 sieq 0
; CHECK:   call @ar.libc.exit(1)
; CHECK:   unreachable
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#2} {
; CHECK:   %.0 sine 0
; CHECK:   si32 %3 = %.0 sadd.nw 1
; CHECK:   si32 %.0 = %3
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "non-term-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!15 = !DILocalVariable(name: "argc", arg: 1, scope: !8, file: !1, line: 3, type: !11)
!16 = !DILocation(line: 0, scope: !8)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !8, file: !1, line: 3, type: !12)
!18 = !DILocation(line: 5, column: 3, scope: !8)
!19 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 4, type: !11)
!20 = !DILocation(line: 6, column: 11, scope: !21)
!21 = distinct !DILexicalBlock(scope: !22, file: !1, line: 6, column: 9)
!22 = distinct !DILexicalBlock(scope: !8, file: !1, line: 5, column: 13)
!23 = !DILocation(line: 6, column: 9, scope: !22)
!24 = !DILocation(line: 7, column: 7, scope: !21)
!25 = !DILocation(line: 8, column: 6, scope: !22)
!26 = distinct !{!26, !18, !27}
!27 = !DILocation(line: 9, column: 3, scope: !8)
