; ModuleID = 'vector-3.pp.bc'
source_filename = "vector-3.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

@a = common global <4 x i32> zeroinitializer, align 16, !dbg !0
; CHECK: define <4 x si32>* @a, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @a, aggregate_zero, align 1
; CHECK: }
; CHECK: }

@b = common global <4 x i32> zeroinitializer, align 16, !dbg !6
; CHECK: define <4 x si32>* @b, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @b, aggregate_zero, align 1
; CHECK: }
; CHECK: }

@c = common global <4 x i32> zeroinitializer, align 16, !dbg !13
; CHECK: define <4 x si32>* @c, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @c, aggregate_zero, align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !20 {
  %1 = load <4 x i32>, <4 x i32>* @a, align 16, !dbg !23
  %2 = load <4 x i32>, <4 x i32>* @b, align 16, !dbg !24
  %3 = add <4 x i32> %1, %2, !dbg !25
  store <4 x i32> %3, <4 x i32>* @c, align 16, !dbg !26
  ret i32 0, !dbg !27
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   <4 x ui32>* %1 = bitcast @a
; CHECK:   <4 x ui32> %2 = load %1, align 16
; CHECK:   <4 x ui32>* %3 = bitcast @b
; CHECK:   <4 x ui32> %4 = load %3, align 16
; CHECK:   <4 x ui32> %5 = %2 uadd %4
; CHECK:   <4 x si32> %6 = bitcast %5
; CHECK:   store @c, %6, align 16
; CHECK:   return 0
; CHECK: }
; CHECK: }

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!15, !16, !17, !18}
!llvm.ident = !{!19}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "vector-3.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!4 = !{}
!5 = !{!0, !6, !13}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "b", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!8 = !DIDerivedType(tag: DW_TAG_typedef, name: "vector_t", file: !3, line: 1, baseType: !9)
!9 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 128, flags: DIFlagVector, elements: !11)
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !{!12}
!12 = !DISubrange(count: 4)
!13 = !DIGlobalVariableExpression(var: !14, expr: !DIExpression())
!14 = distinct !DIGlobalVariable(name: "c", scope: !2, file: !3, line: 3, type: !8, isLocal: false, isDefinition: true)
!15 = !{i32 2, !"Dwarf Version", i32 4}
!16 = !{i32 2, !"Debug Info Version", i32 3}
!17 = !{i32 1, !"wchar_size", i32 4}
!18 = !{i32 7, !"PIC Level", i32 2}
!19 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!20 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 5, type: !21, scopeLine: 5, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!21 = !DISubroutineType(types: !22)
!22 = !{!10}
!23 = !DILocation(line: 6, column: 7, scope: !20)
!24 = !DILocation(line: 6, column: 11, scope: !20)
!25 = !DILocation(line: 6, column: 9, scope: !20)
!26 = !DILocation(line: 6, column: 5, scope: !20)
!27 = !DILocation(line: 7, column: 3, scope: !20)
