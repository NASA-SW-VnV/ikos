; ModuleID = 'bit-field.c.pp.bc'
source_filename = "bit-field.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

%struct.info_t = type { i16, [2 x i8] }

@main.info = private unnamed_addr constant { i8, i8, [2 x i8] } { i8 -88, i8 16, [2 x i8] undef }, align 4
; CHECK: define {0: si8, 1: si8, 2: [2 x si8]}* @main.info, align 4, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @main.info, {0: -88, 1: 16, 2: undef}, align 1
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #2
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !7 {
  %1 = alloca %struct.info_t, align 4
  call void @llvm.dbg.declare(metadata %struct.info_t* %1, metadata !11, metadata !19), !dbg !20
  %2 = bitcast %struct.info_t* %1 to i8*, !dbg !20
  %3 = getelementptr inbounds { i8, i8, [2 x i8] }, { i8, i8, [2 x i8] }* @main.info, i32 0, i32 0, !dbg !20
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %2, i8* %3, i64 4, i32 4, i1 false), !dbg !20
  ret i32 0, !dbg !21
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: ui16, 2: [2 x si8]}* $1 = allocate {0: ui16, 2: [2 x si8]}, 1, align 4
; CHECK:   si8* %2 = bitcast $1
; CHECK:   si8* %3 = ptrshift @main.info, 4 * 0, 1 * 0
; CHECK:   call @ar.memcpy(%2, %3, 4, 4, 0)
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
!1 = !DIFile(filename: "bit-field.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 8, type: !8, isLocal: false, isDefinition: true, scopeLine: 8, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "info", scope: !7, file: !1, line: 9, type: !12)
!12 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "info_t", file: !1, line: 1, size: 32, elements: !13)
!13 = !{!14, !16, !17, !18}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !12, file: !1, line: 2, baseType: !15, size: 1, flags: DIFlagBitField, extraData: i64 0)
!15 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!16 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !12, file: !1, line: 3, baseType: !15, size: 1, offset: 1, flags: DIFlagBitField, extraData: i64 0)
!17 = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !12, file: !1, line: 4, baseType: !10, size: 10, offset: 2, flags: DIFlagBitField, extraData: i64 0)
!18 = !DIDerivedType(tag: DW_TAG_member, name: "k", scope: !12, file: !1, line: 5, baseType: !10, size: 1, offset: 12, flags: DIFlagBitField, extraData: i64 0)
!19 = !DIExpression()
!20 = !DILocation(line: 9, column: 17, scope: !7)
!21 = !DILocation(line: 10, column: 3, scope: !7)
