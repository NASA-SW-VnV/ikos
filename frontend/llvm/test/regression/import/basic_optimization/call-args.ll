; ModuleID = 'call-args.c.pp.bc'
source_filename = "call-args.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @foo(i32) #0 !dbg !7 {
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !11, metadata !12), !dbg !13
  %2 = add nsw i32 %0, 1, !dbg !14
  ret i32 %2, !dbg !15
}
; CHECK: define si32 @foo(si32 %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 %2 = %1 sadd.nw 1
; CHECK:   return %2
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !16 {
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !22, metadata !12), !dbg !23
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !24, metadata !12), !dbg !25
  %3 = call i32 @foo(i32 %0), !dbg !26
  ret i32 %3, !dbg !27
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 %3 = call @foo(%1)
; CHECK:   return %3
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "call-args.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "i", arg: 1, scope: !7, file: !1, line: 1, type: !10)
!12 = !DIExpression()
!13 = !DILocation(line: 1, column: 13, scope: !7)
!14 = !DILocation(line: 2, column: 12, scope: !7)
!15 = !DILocation(line: 2, column: 3, scope: !7)
!16 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 5, type: !17, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!17 = !DISubroutineType(types: !18)
!18 = !{!10, !10, !19}
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !21, size: 64)
!21 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!22 = !DILocalVariable(name: "argc", arg: 1, scope: !16, file: !1, line: 5, type: !10)
!23 = !DILocation(line: 5, column: 14, scope: !16)
!24 = !DILocalVariable(name: "argv", arg: 2, scope: !16, file: !1, line: 5, type: !19)
!25 = !DILocation(line: 5, column: 27, scope: !16)
!26 = !DILocation(line: 6, column: 10, scope: !16)
!27 = !DILocation(line: 6, column: 3, scope: !16)
