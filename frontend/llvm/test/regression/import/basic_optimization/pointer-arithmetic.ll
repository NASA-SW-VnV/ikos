; ModuleID = 'pointer-arithmetic.pp.bc'
source_filename = "pointer-arithmetic.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.vector = type { i32, i32, i32 }

@.str = private unnamed_addr constant [4 x i8] c"aaa\00", align 1
; CHECK: define [4 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [97, 97, 97, 0], align 1
; CHECK: }
; CHECK: }

@.str.1 = private unnamed_addr constant [4 x i8] c"bbb\00", align 1
; CHECK: define [4 x si8]* @.str.1, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str.1, [98, 98, 98, 0], align 1
; CHECK: }
; CHECK: }

@.str.2 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; CHECK: define [4 x si8]* @.str.2, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str.2, [37, 100, 10, 0], align 1
; CHECK: }
; CHECK: }

@ptr = global i32* bitcast (i8* getelementptr (i8, i8* bitcast ([2 x %struct.vector]* @v to i8*), i64 20) to i32*), align 8, !dbg !23
; CHECK: define si32** @ptr, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @v
; CHECK:   si8* %2 = ptrshift %1, 1 * 20
; CHECK:   si32* %3 = bitcast %2
; CHECK:   store @ptr, %3, align 1
; CHECK: }
; CHECK: }

@ptr_fun = global i8* getelementptr (i8, i8* bitcast (i32 ()* @_Z1fv to i8*), i64 1), align 8, !dbg !0
; CHECK: define ui8** @ptr_fun, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_Z1fv
; CHECK:   ui8* %2 = ptrshift %1, 1 * 1
; CHECK:   store @ptr_fun, %2, align 1
; CHECK: }
; CHECK: }

@string_map = global [2 x i8*] [i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.1, i32 0, i32 0)], align 16, !dbg !6
; CHECK: define [2 x si8*]* @string_map, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = ptrshift @.str.1, 4 * 0, 1 * 0
; CHECK:   si8* %2 = ptrshift @.str, 4 * 0, 1 * 0
; CHECK:   store @string_map, [%2, %1], align 1
; CHECK: }
; CHECK: }

@v = global [2 x %struct.vector] [%struct.vector { i32 1, i32 2, i32 3 }, %struct.vector { i32 4, i32 5, i32 6 }], align 16, !dbg !14
; CHECK: define [2 x {0: si32, 4: si32, 8: si32}]* @v, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @v, [{0: 1, 4: 2, 8: 3}, {0: 4, 4: 5, 8: 6}], align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @_Z1fv() #0 !dbg !33 {
  ret i32 6, !dbg !36
}
; CHECK: define si32 @_Z1fv() {
; CHECK: #1 !entry !exit {
; CHECK:   return 6
; CHECK: }
; CHECK: }

declare i32 @printf(i8*, ...) #2
; CHECK: declare si32 @ar.libc.printf(si8*, ...)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main() #1 !dbg !37 {
  %1 = getelementptr inbounds [2 x %struct.vector], [2 x %struct.vector]* @v, i64 0, i64 1, i32 2, !dbg !38
  %2 = load i32, i32* %1, align 4, !dbg !38
  %3 = getelementptr inbounds [4 x i8], [4 x i8]* @.str.2, i64 0, i64 0, !dbg !39
  %4 = call i32 (i8*, ...) @printf(i8* %3, i32 %2), !dbg !39
  ret i32 %4, !dbg !40
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* %1 = ptrshift @v, 24 * 0, 12 * 1, 1 * 8
; CHECK:   si32 %2 = load %1, align 4
; CHECK:   si8* %3 = ptrshift @.str.2, 4 * 0, 1 * 0
; CHECK:   si32 %4 = call @ar.libc.printf(%3, %2)
; CHECK:   return %4
; CHECK: }
; CHECK: }

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!28, !29, !30, !31}
!llvm.ident = !{!32}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "ptr_fun", scope: !2, file: !3, line: 7, type: !26, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "pointer-arithmetic.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!4 = !{}
!5 = !{!0, !6, !14, !23}
!6 = !DIGlobalVariableExpression(var: !7, expr: !DIExpression())
!7 = distinct !DIGlobalVariable(name: "string_map", scope: !2, file: !3, line: 9, type: !8, isLocal: false, isDefinition: true)
!8 = !DICompositeType(tag: DW_TAG_array_type, baseType: !9, size: 128, elements: !12)
!9 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!10 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !11)
!11 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!12 = !{!13}
!13 = !DISubrange(count: 2)
!14 = !DIGlobalVariableExpression(var: !15, expr: !DIExpression())
!15 = distinct !DIGlobalVariable(name: "v", scope: !2, file: !3, line: 17, type: !16, isLocal: false, isDefinition: true)
!16 = !DICompositeType(tag: DW_TAG_array_type, baseType: !17, size: 192, elements: !12)
!17 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "vector", file: !3, line: 11, size: 96, flags: DIFlagTypePassByValue, elements: !18, identifier: "_ZTS6vector")
!18 = !{!19, !21, !22}
!19 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !17, file: !3, line: 12, baseType: !20, size: 32)
!20 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!21 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !17, file: !3, line: 13, baseType: !20, size: 32, offset: 32)
!22 = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !17, file: !3, line: 14, baseType: !20, size: 32, offset: 64)
!23 = !DIGlobalVariableExpression(var: !24, expr: !DIExpression())
!24 = distinct !DIGlobalVariable(name: "ptr", scope: !2, file: !3, line: 19, type: !25, isLocal: false, isDefinition: true)
!25 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!26 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !27, size: 64)
!27 = !DIBasicType(name: "unsigned char", size: 8, encoding: DW_ATE_unsigned_char)
!28 = !{i32 2, !"Dwarf Version", i32 4}
!29 = !{i32 2, !"Debug Info Version", i32 3}
!30 = !{i32 1, !"wchar_size", i32 4}
!31 = !{i32 7, !"PIC Level", i32 2}
!32 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!33 = distinct !DISubprogram(name: "f", linkageName: "_Z1fv", scope: !3, file: !3, line: 3, type: !34, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!34 = !DISubroutineType(types: !35)
!35 = !{!20}
!36 = !DILocation(line: 4, column: 3, scope: !33)
!37 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 21, type: !34, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!38 = !DILocation(line: 22, column: 30, scope: !37)
!39 = !DILocation(line: 22, column: 10, scope: !37)
!40 = !DILocation(line: 22, column: 3, scope: !37)
