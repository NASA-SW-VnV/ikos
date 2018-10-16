; ModuleID = 'phi-1.c.pp.bc'
source_filename = "phi-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !14 {
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !21, metadata !22), !dbg !23
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !24, metadata !22), !dbg !25
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !26, metadata !22), !dbg !27
  ret i32 0, !dbg !28
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!10, !11, !12}
!llvm.ident = !{!13}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3)
!1 = !DIFile(filename: "phi-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4}
!4 = !DIGlobalVariableExpression(var: !5)
!5 = distinct !DIGlobalVariable(name: "a", scope: !0, file: !1, line: 1, type: !6, isLocal: false, isDefinition: true)
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 640, elements: !8)
!7 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!8 = !{!9}
!9 = !DISubrange(count: 10)
!10 = !{i32 2, !"Dwarf Version", i32 4}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 1, !"PIC Level", i32 2}
!13 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!14 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !15, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!15 = !DISubroutineType(types: !16)
!16 = !{!17, !17, !18}
!17 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!20 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!21 = !DILocalVariable(name: "argc", arg: 1, scope: !14, file: !1, line: 3, type: !17)
!22 = !DIExpression()
!23 = !DILocation(line: 3, column: 14, scope: !14)
!24 = !DILocalVariable(name: "argv", arg: 2, scope: !14, file: !1, line: 3, type: !18)
!25 = !DILocation(line: 3, column: 27, scope: !14)
!26 = !DILocalVariable(name: "i", scope: !14, file: !1, line: 4, type: !17)
!27 = !DILocation(line: 4, column: 7, scope: !14)
!28 = !DILocation(line: 9, column: 1, scope: !14)
