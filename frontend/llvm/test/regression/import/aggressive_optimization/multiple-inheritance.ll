; ModuleID = 'multiple-inheritance.pp.bc'
source_filename = "multiple-inheritance.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

; Function Attrs: noinline norecurse nounwind ssp uwtable
define i32 @main() local_unnamed_addr #0 !dbg !35 {
  ret i32 0, !dbg !38
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   return 0
; CHECK: }
; CHECK: }

attributes #0 = { noinline norecurse nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!30, !31, !32, !33}
!llvm.ident = !{!34}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "multiple-inheritance.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4}
!4 = !DIGlobalVariableExpression(var: !5, expr: !DIExpression())
!5 = distinct !DIGlobalVariable(name: "c", scope: !0, file: !1, line: 21, type: !6, isLocal: false, isDefinition: true)
!6 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Child", file: !1, line: 16, size: 416, flags: DIFlagTypePassByValue, elements: !7, identifier: "_ZTS5Child")
!7 = !{!8, !14, !19, !26}
!8 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !6, baseType: !9, flags: DIFlagPublic, extraData: i32 0)
!9 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "Base", file: !1, line: 1, size: 64, flags: DIFlagTypePassByValue, elements: !10, identifier: "_ZTS4Base")
!10 = !{!11, !13}
!11 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !9, file: !1, line: 3, baseType: !12, size: 32)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !9, file: !1, line: 4, baseType: !12, size: 32, offset: 32)
!14 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !6, baseType: !15, offset: 64, flags: DIFlagPublic, extraData: i32 0)
!15 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Mixin", file: !1, line: 7, size: 32, flags: DIFlagTypePassByValue, elements: !16, identifier: "_ZTS5Mixin")
!16 = !{!17}
!17 = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !15, file: !1, line: 9, baseType: !18, size: 32, flags: DIFlagPrivate)
!18 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!19 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !6, baseType: !20, flags: DIFlagPublic, extraData: i32 0)
!20 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "Empty", file: !1, line: 12, size: 8, flags: DIFlagTypePassByValue, elements: !21, identifier: "_ZTS5Empty")
!21 = !{!22}
!22 = !DISubprogram(name: "f", linkageName: "_ZN5Empty1fEv", scope: !20, file: !1, line: 13, type: !23, scopeLine: 13, flags: DIFlagPrototyped, spFlags: 0)
!23 = !DISubroutineType(types: !24)
!24 = !{null, !25}
!25 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!26 = !DIDerivedType(tag: DW_TAG_member, name: "tab", scope: !6, file: !1, line: 18, baseType: !27, size: 320, offset: 96)
!27 = !DICompositeType(tag: DW_TAG_array_type, baseType: !12, size: 320, elements: !28)
!28 = !{!29}
!29 = !DISubrange(count: 10)
!30 = !{i32 2, !"Dwarf Version", i32 4}
!31 = !{i32 2, !"Debug Info Version", i32 3}
!32 = !{i32 1, !"wchar_size", i32 4}
!33 = !{i32 7, !"PIC Level", i32 2}
!34 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!35 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 23, type: !36, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!36 = !DISubroutineType(types: !37)
!37 = !{!12}
!38 = !DILocation(line: 24, column: 3, scope: !35)
