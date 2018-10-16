; ModuleID = 'reference.cpp.pp.bc'
source_filename = "reference.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_Z1fRi(i32* dereferenceable(4)) unnamed_addr #0 !dbg !7 {
  call void @llvm.dbg.value(metadata i32* %0, i64 0, metadata !12, metadata !13), !dbg !14
  store i32 1, i32* %0, align 4, !dbg !15
  ret void, !dbg !16
}
; CHECK: define void @_Z1fRi(si32* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   store %1, 1, align 4
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline norecurse nounwind ssp uwtable
define i32 @main() local_unnamed_addr #1 !dbg !17 {
  %1 = alloca i32, align 4
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !20, metadata !13), !dbg !21
  store i32 0, i32* %1, align 4, !dbg !21
  call void @llvm.dbg.value(metadata i32* %1, i64 0, metadata !20, metadata !22), !dbg !21
  call fastcc void @_Z1fRi(i32* nonnull dereferenceable(4) %1), !dbg !23
  %2 = load i32, i32* %1, align 4, !dbg !24
  call void @llvm.dbg.value(metadata i32 %2, i64 0, metadata !20, metadata !13), !dbg !21
  ret i32 %2, !dbg !25
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   store $1, 0, align 4
; CHECK:   call @_Z1fRi($1)
; CHECK:   si32 %2 = load $1, align 4
; CHECK:   return %2
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #2

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline norecurse nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "reference.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "f", linkageName: "_Z1fRi", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{null, !10}
!10 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !11, size: 64)
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "x", arg: 1, scope: !7, file: !1, line: 1, type: !10)
!13 = !DIExpression()
!14 = !DILocation(line: 1, column: 13, scope: !7)
!15 = !DILocation(line: 2, column: 5, scope: !7)
!16 = !DILocation(line: 3, column: 1, scope: !7)
!17 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 5, type: !18, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!18 = !DISubroutineType(types: !19)
!19 = !{!11}
!20 = !DILocalVariable(name: "y", scope: !17, file: !1, line: 6, type: !11)
!21 = !DILocation(line: 6, column: 7, scope: !17)
!22 = !DIExpression(DW_OP_deref)
!23 = !DILocation(line: 7, column: 3, scope: !17)
!24 = !DILocation(line: 8, column: 10, scope: !17)
!25 = !DILocation(line: 8, column: 3, scope: !17)
