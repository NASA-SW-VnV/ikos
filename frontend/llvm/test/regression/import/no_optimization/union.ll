; ModuleID = 'union.c.pp.bc'
source_filename = "union.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

%union.my_union = type { i8* }

@main.x = private unnamed_addr constant { i32, [4 x i8] } { i32 1, [4 x i8] undef }, align 8
; CHECK: define {0: si32, 4: [4 x si8]}* @main.x, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @main.x, {0: 1, 4: undef}, align 1
; CHECK: }
; CHECK: }

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #2
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !7 {
  %1 = alloca %union.my_union, align 8
  call void @llvm.dbg.declare(metadata %union.my_union* %1, metadata !11, metadata !18), !dbg !19
  %2 = bitcast %union.my_union* %1 to i8*, !dbg !19
  %3 = bitcast { i32, [4 x i8] }* @main.x to i8*, !dbg !19
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %2, i8* %3, i64 8, i32 8, i1 false), !dbg !19
  ret i32 0, !dbg !20
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si8*}* $1 = allocate {0: si8*}, 1, align 8
; CHECK:   si8* %2 = bitcast $1
; CHECK:   si8* %3 = bitcast @main.x
; CHECK:   call @ar.memcpy(%2, %3, 8, 8, 0)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "union.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !8, isLocal: false, isDefinition: true, scopeLine: 6, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "x", scope: !7, file: !1, line: 7, type: !12)
!12 = distinct !DICompositeType(tag: DW_TAG_union_type, name: "my_union", file: !1, line: 1, size: 64, elements: !13)
!13 = !{!14, !15}
!14 = !DIDerivedType(tag: DW_TAG_member, name: "m_int", scope: !12, file: !1, line: 2, baseType: !10, size: 32)
!15 = !DIDerivedType(tag: DW_TAG_member, name: "m_ptr", scope: !12, file: !1, line: 3, baseType: !16, size: 64)
!16 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !17, size: 64)
!17 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!18 = !DIExpression()
!19 = !DILocation(line: 7, column: 18, scope: !7)
!20 = !DILocation(line: 8, column: 1, scope: !7)
