; ModuleID = 'opaque-struct.c.pp.bc'
source_filename = "opaque-struct.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

%struct.my_struct = type { i32*, %struct.X* }
%struct.X = type opaque

@s = common global %struct.my_struct zeroinitializer, align 8, !dbg !0
; CHECK: define {0: si32*, 8: opaque*}* @s, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @s, aggregate_zero, align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !19 {
  ret i32 0, !dbg !22
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   return 0
; CHECK: }
; CHECK: }

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!15, !16, !17}
!llvm.ident = !{!18}

!0 = !DIGlobalVariableExpression(var: !1)
!1 = distinct !DIGlobalVariable(name: "s", scope: !2, file: !3, line: 10, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "opaque-struct.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
!4 = !{}
!5 = !{!0}
!6 = !DIDerivedType(tag: DW_TAG_typedef, name: "my_struct", file: !3, line: 8, baseType: !7)
!7 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !3, line: 5, size: 128, elements: !8)
!8 = !{!9, !12}
!9 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !7, file: !3, line: 6, baseType: !10, size: 64)
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_member, name: "b", scope: !7, file: !3, line: 7, baseType: !13, size: 64, offset: 64)
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DICompositeType(tag: DW_TAG_structure_type, name: "X", file: !3, line: 3, flags: DIFlagFwdDecl)
!15 = !{i32 2, !"Dwarf Version", i32 4}
!16 = !{i32 2, !"Debug Info Version", i32 3}
!17 = !{i32 1, !"PIC Level", i32 2}
!18 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!19 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 12, type: !20, isLocal: false, isDefinition: true, scopeLine: 12, isOptimized: false, unit: !2, variables: !4)
!20 = !DISubroutineType(types: !21)
!21 = !{!11}
!22 = !DILocation(line: 13, column: 3, scope: !19)
