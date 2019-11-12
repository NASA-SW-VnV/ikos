; ModuleID = 'union.pp.bc'
source_filename = "union.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%union.my_union = type { i8* }

@__const.main.x = private unnamed_addr constant { i32, [4 x i8] } { i32 1, [4 x i8] undef }, align 8
; CHECK: define {0: si32, 4: [4 x si8]}* @__const.main.x, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @__const.main.x, {0: 1, 4: undef}, align 1
; CHECK: }
; CHECK: }

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #2
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !8 {
  %1 = alloca %union.my_union, align 8
  call void @llvm.dbg.declare(metadata %union.my_union* %1, metadata !12, metadata !DIExpression()), !dbg !19
  %2 = bitcast %union.my_union* %1 to i8*, !dbg !19
  %3 = bitcast { i32, [4 x i8] }* @__const.main.x to i8*, !dbg !19
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %2, i8* align 8 %3, i64 8, i1 false), !dbg !19
  ret i32 0, !dbg !20
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si8*}* $1 = allocate {0: si8*}, 1, align 8
; CHECK:   si8* %2 = bitcast $1
; CHECK:   si8* %3 = bitcast @__const.main.x
; CHECK:   call @ar.memcpy(%2, %3, 8, 8, 8, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "union.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !9, scopeLine: 6, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "x", scope: !8, file: !1, line: 7, type: !13)
!13 = distinct !DICompositeType(tag: DW_TAG_union_type, name: "my_union", file: !1, line: 1, size: 64, elements: !14)
!14 = !{!15, !16}
!15 = !DIDerivedType(tag: DW_TAG_member, name: "m_int", scope: !13, file: !1, line: 2, baseType: !11, size: 32)
!16 = !DIDerivedType(tag: DW_TAG_member, name: "m_ptr", scope: !13, file: !1, line: 3, baseType: !17, size: 64)
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !18, size: 64)
!18 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!19 = !DILocation(line: 7, column: 18, scope: !8)
!20 = !DILocation(line: 8, column: 1, scope: !8)
