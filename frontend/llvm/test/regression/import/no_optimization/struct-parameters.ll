; ModuleID = 'struct-parameters.c.pp.bc'
source_filename = "struct-parameters.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

%struct.my_struct = type { [10 x i8], [10 x i8], [10 x i8] }

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #2
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define void @f(%struct.my_struct* noalias sret, %struct.my_struct*) #0 !dbg !7 {
  %3 = alloca %struct.my_struct*, align 8
  store %struct.my_struct* %1, %struct.my_struct** %3, align 8
  call void @llvm.dbg.declare(metadata %struct.my_struct** %3, metadata !21, metadata !22), !dbg !23
  %4 = load %struct.my_struct*, %struct.my_struct** %3, align 8, !dbg !24
  %5 = bitcast %struct.my_struct* %0 to i8*, !dbg !25
  %6 = bitcast %struct.my_struct* %4 to i8*, !dbg !25
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %5, i8* %6, i64 30, i32 1, i1 false), !dbg !25
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
; CHECK:   call @ar.memcpy(%6, %7, 30, 1, 0)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define void @g(%struct.my_struct* noalias sret, %struct.my_struct* byval align 8) #0 !dbg !27 {
  call void @llvm.dbg.declare(metadata %struct.my_struct* %1, metadata !30, metadata !22), !dbg !31
  %3 = bitcast %struct.my_struct* %0 to i8*, !dbg !32
  %4 = bitcast %struct.my_struct* %1 to i8*, !dbg !32
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %3, i8* %4, i64 30, i32 1, i1 false), !dbg !32
  ret void, !dbg !33
}
; CHECK: define void @g({0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}* %1, {0: [10 x si8], 10: [10 x si8], 20: [10 x si8]}* %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %3 = bitcast %1
; CHECK:   si8* %4 = bitcast %2
; CHECK:   call @ar.memcpy(%3, %4, 30, 1, 0)
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

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "struct-parameters.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "f", scope: !1, file: !1, line: 9, type: !8, isLocal: false, isDefinition: true, scopeLine: 9, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !20}
!10 = !DIDerivedType(tag: DW_TAG_typedef, name: "my_struct", file: !1, line: 7, baseType: !11)
!11 = distinct !DICompositeType(tag: DW_TAG_structure_type, file: !1, line: 3, size: 240, elements: !12)
!12 = !{!13, !18, !19}
!13 = !DIDerivedType(tag: DW_TAG_member, name: "buf", scope: !11, file: !1, line: 4, baseType: !14, size: 80)
!14 = !DICompositeType(tag: DW_TAG_array_type, baseType: !15, size: 80, elements: !16)
!15 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!16 = !{!17}
!17 = !DISubrange(count: 10)
!18 = !DIDerivedType(tag: DW_TAG_member, name: "buf1", scope: !11, file: !1, line: 5, baseType: !14, size: 80, offset: 80)
!19 = !DIDerivedType(tag: DW_TAG_member, name: "buf2", scope: !11, file: !1, line: 6, baseType: !14, size: 80, offset: 160)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!21 = !DILocalVariable(name: "s", arg: 1, scope: !7, file: !1, line: 9, type: !20)
!22 = !DIExpression()
!23 = !DILocation(line: 9, column: 24, scope: !7)
!24 = !DILocation(line: 10, column: 11, scope: !7)
!25 = !DILocation(line: 10, column: 10, scope: !7)
!26 = !DILocation(line: 10, column: 3, scope: !7)
!27 = distinct !DISubprogram(name: "g", scope: !1, file: !1, line: 13, type: !28, isLocal: false, isDefinition: true, scopeLine: 13, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!28 = !DISubroutineType(types: !29)
!29 = !{!10, !10}
!30 = !DILocalVariable(name: "s", arg: 1, scope: !27, file: !1, line: 13, type: !10)
!31 = !DILocation(line: 13, column: 23, scope: !27)
!32 = !DILocation(line: 14, column: 10, scope: !27)
!33 = !DILocation(line: 14, column: 3, scope: !27)
!34 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 17, type: !35, isLocal: false, isDefinition: true, scopeLine: 17, isOptimized: false, unit: !0, variables: !2)
!35 = !DISubroutineType(types: !36)
!36 = !{!37}
!37 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!38 = !DILocation(line: 18, column: 3, scope: !34)
