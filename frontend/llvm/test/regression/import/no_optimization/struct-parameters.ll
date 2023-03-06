; ModuleID = 'struct-parameters.pp.bc'
source_filename = "struct-parameters.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.my_struct = type { [10 x i8], [10 x i8], [10 x i8] }

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1) #2
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define void @f(%struct.my_struct* noalias sret(%struct.my_struct), %struct.my_struct*) #0 !dbg !8 {
  %3 = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %1, %struct.my_struct** %3, align 8
  call void @llvm.dbg.declare(metadata %struct.my_struct** %3, metadata !22, metadata !DIExpression()), !dbg !23
  %4 = load %struct.my_struct*, %struct.my_struct** %3, align 8, !dbg !24
  %5 = bitcast %struct.my_struct* %0 to i8*, !dbg !25
  %6 = bitcast %struct.my_struct* %4 to i8*, !dbg !25
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %5, i8* align 1 %6, i64 30, i1 false), !dbg !25
  ret void, !dbg !26
}
; CHECK: define void @f({0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}* %1, {0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}* %2) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}** $3 = allocate {0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}*, 1, align 8
; CHECK:   store $3, %2, align 8
; CHECK:   {0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}** %4 = bitcast $3
; CHECK:   {0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}* %5 = load %4, align 8
; CHECK:   si8* %6 = bitcast %1
; CHECK:   si8* %7 = bitcast %5
; CHECK:   call @ar.memcpy(%6, %7, 30, 1, 1, 0)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define void @g(%struct.my_struct* noalias sret(%struct.my_struct), %struct.my_struct* byval(%struct.my_struct) align 8) #0 !dbg !27 {
  call void @llvm.dbg.declare(metadata %struct.my_struct* %1, metadata !30, metadata !DIExpression()), !dbg !31
  %3 = bitcast %struct.my_struct* %0 to i8*, !dbg !32
  %4 = bitcast %struct.my_struct* %1 to i8*, !dbg !32
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 1 %3, i8* align 8 %4, i64 30, i1 false), !dbg !32
  ret void, !dbg !33
}
; CHECK: define void @g({0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}* %1, {0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}* %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %3 = bitcast %1
; CHECK:   si8* %4 = bitcast %2
; CHECK:   call @ar.memcpy(%3, %4, 30, 1, 8, 0)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !34 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  ret i32 0, !dbg !38
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   store $1, 0, align 4
; CHECK:   return 0
; CHECK: }
; CHECK: }

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "struct-parameters.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 9, type: !9, scopeLine: 9, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !21}
!11 = !DIDerivedType(tag: DW_TAG_typedef, name: "my_struct", file: !1, line: 7, baseType: !12)
!12 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1, line: 3, size: 240, elements: !13)
!13 = !{!14, !19, !20}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "buf", scope: !12, file: !1, line: 4, baseType: !15, size: 80)
!15 = !DICompositeType(tag: DW_TAG_array_type, baseType: !16, size: 80, elements: !17)
!16 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!17 = !{!18}
!18 = !DISubrange(count: 10)
!19 = !DIDerivedType(tag: DW_TAG_member, name: "buf1", scope: !12, file: !1, line: 5, baseType: !15, size: 80, offset: 80)
!20 = !DIDerivedType(tag: DW_TAG_member, name: "buf2", scope: !12, file: !1, line: 6, baseType: !15, size: 80, offset: 160)
!21 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!22 = !DILocalVariable(name: "s", arg: 1, scope: !8, file: !1, line: 9, type: !21)
!23 = !DILocation(line: 9, column: 24, scope: !8)
!24 = !DILocation(line: 10, column: 11, scope: !8)
!25 = !DILocation(line: 10, column: 10, scope: !8)
!26 = !DILocation(line: 10, column: 3, scope: !8)
!27 = distinct !DISubprogram(name: "g", scope: !1, file: !1, line: 13, type: !28, scopeLine: 13, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!28 = !DISubroutineType(types: !29)
!29 = !{!11, !11}
!30 = !DILocalVariable(name: "s", arg: 1, scope: !27, file: !1, line: 13, type: !11)
!31 = !DILocation(line: 13, column: 23, scope: !27)
!32 = !DILocation(line: 14, column: 10, scope: !27)
!33 = !DILocation(line: 14, column: 3, scope: !27)
!34 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 17, type: !35, scopeLine: 17, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!35 = !DISubroutineType(types: !36)
!36 = !{!37}
!37 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!38 = !DILocation(line: 18, column: 3, scope: !34)
