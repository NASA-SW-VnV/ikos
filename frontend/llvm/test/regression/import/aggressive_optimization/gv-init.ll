; ModuleID = 'gv-init.c.pp.bc'
source_filename = "gv-init.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) local_unnamed_addr #0 !dbg !23 {
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !29, metadata !30), !dbg !31
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !32, metadata !30), !dbg !33
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !34, metadata !30), !dbg !35
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !36, metadata !30), !dbg !37
  br label %3, !dbg !38

; <label>:3:                                      ; preds = %8, %2
  %.01 = phi i32 [ 0, %2 ], [ %9, %8 ]
  %.0 = phi i32 [ 0, %2 ], [ %.1.lcssa, %8 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !36, metadata !30), !dbg !37
  call void @llvm.dbg.value(metadata i32 %.01, i64 0, metadata !34, metadata !30), !dbg !35
  %4 = icmp slt i32 %.01, 100, !dbg !39
  br i1 %4, label %.preheader, label %10, !dbg !43

.preheader:                                       ; preds = %3, %6
  %.1 = phi i32 [ %7, %6 ], [ %.0, %3 ]
  call void @llvm.dbg.value(metadata i32 %.1, i64 0, metadata !36, metadata !30), !dbg !37
  %5 = icmp slt i32 %.1, 100, !dbg !45
  br i1 %5, label %6, label %8, !dbg !49

; <label>:6:                                      ; preds = %.preheader
  %7 = add nsw i32 %.1, 1, !dbg !51
  call void @llvm.dbg.value(metadata i32 %7, i64 0, metadata !36, metadata !30), !dbg !37
  br label %.preheader, !dbg !53, !llvm.loop !54

; <label>:8:                                      ; preds = %.preheader
  %.1.lcssa = phi i32 [ %.1, %.preheader ]
  %9 = add nsw i32 %.01, 1, !dbg !57
  call void @llvm.dbg.value(metadata i32 %9, i64 0, metadata !34, metadata !30), !dbg !35
  br label %3, !dbg !59, !llvm.loop !60

; <label>:10:                                     ; preds = %3
  ret i32 0, !dbg !63
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32 %.01 = 0
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #6} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#.preheader} {
; CHECK:   %.01 silt 100
; CHECK:   si32 %.1 = %.0
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %.01 sige 100
; CHECK:   return 0
; CHECK: }
; CHECK: #.preheader predecessors={#3, #5} successors={#5, #6} {
; CHECK: }
; CHECK: #5 predecessors={#.preheader} successors={#.preheader} {
; CHECK:   %.1 silt 100
; CHECK:   si32 %3 = %.1 sadd.nw 1
; CHECK:   si32 %.1 = %3
; CHECK: }
; CHECK: #6 predecessors={#.preheader} successors={#2} {
; CHECK:   %.1 sige 100
; CHECK:   si32 %.1.lcssa = %.1
; CHECK:   si32 %4 = %.01 sadd.nw 1
; CHECK:   si32 %.01 = %4
; CHECK:   si32 %.0 = %.1.lcssa
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!19, !20, !21}
!llvm.ident = !{!22}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3)
!1 = !DIFile(filename: "gv-init.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4, !10, !12, !17}
!4 = !DIGlobalVariableExpression(var: !5)
!5 = distinct !DIGlobalVariable(name: "b", scope: !0, file: !1, line: 2, type: !6, isLocal: false, isDefinition: true)
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 64, elements: !8)
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{!9}
!9 = !DISubrange(count: 2)
!10 = !DIGlobalVariableExpression(var: !11)
!11 = distinct !DIGlobalVariable(name: "d", scope: !0, file: !1, line: 4, type: !7, isLocal: false, isDefinition: true)
!12 = !DIGlobalVariableExpression(var: !13)
!13 = distinct !DIGlobalVariable(name: "a", scope: !0, file: !1, line: 1, type: !14, isLocal: false, isDefinition: true)
!14 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 320000, elements: !15)
!15 = !{!16, !16}
!16 = !DISubrange(count: 100)
!17 = !DIGlobalVariableExpression(var: !18)
!18 = distinct !DIGlobalVariable(name: "c", scope: !0, file: !1, line: 3, type: !7, isLocal: false, isDefinition: true)
!19 = !{i32 2, !"Dwarf Version", i32 4}
!20 = !{i32 2, !"Debug Info Version", i32 3}
!21 = !{i32 1, !"PIC Level", i32 2}
!22 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!23 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 7, type: !24, isLocal: false, isDefinition: true, scopeLine: 7, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!24 = !DISubroutineType(types: !25)
!25 = !{!7, !7, !26}
!26 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !27, size: 64)
!27 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !28, size: 64)
!28 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!29 = !DILocalVariable(name: "argc", arg: 1, scope: !23, file: !1, line: 7, type: !7)
!30 = !DIExpression()
!31 = !DILocation(line: 7, column: 14, scope: !23)
!32 = !DILocalVariable(name: "argv", arg: 2, scope: !23, file: !1, line: 7, type: !26)
!33 = !DILocation(line: 7, column: 27, scope: !23)
!34 = !DILocalVariable(name: "i", scope: !23, file: !1, line: 8, type: !7)
!35 = !DILocation(line: 8, column: 7, scope: !23)
!36 = !DILocalVariable(name: "j", scope: !23, file: !1, line: 8, type: !7)
!37 = !DILocation(line: 8, column: 14, scope: !23)
!38 = !DILocation(line: 9, column: 3, scope: !23)
!39 = !DILocation(line: 9, column: 12, scope: !40)
!40 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 1)
!41 = distinct !DILexicalBlock(scope: !42, file: !1, line: 9, column: 3)
!42 = distinct !DILexicalBlock(scope: !23, file: !1, line: 9, column: 3)
!43 = !DILocation(line: 9, column: 3, scope: !44)
!44 = !DILexicalBlockFile(scope: !42, file: !1, discriminator: 1)
!45 = !DILocation(line: 10, column: 14, scope: !46)
!46 = !DILexicalBlockFile(scope: !47, file: !1, discriminator: 1)
!47 = distinct !DILexicalBlock(scope: !48, file: !1, line: 10, column: 5)
!48 = distinct !DILexicalBlock(scope: !41, file: !1, line: 10, column: 5)
!49 = !DILocation(line: 10, column: 5, scope: !50)
!50 = !DILexicalBlockFile(scope: !48, file: !1, discriminator: 1)
!51 = !DILocation(line: 10, column: 22, scope: !52)
!52 = !DILexicalBlockFile(scope: !47, file: !1, discriminator: 2)
!53 = !DILocation(line: 10, column: 5, scope: !52)
!54 = distinct !{!54, !55, !56}
!55 = !DILocation(line: 10, column: 5, scope: !48)
!56 = !DILocation(line: 14, column: 30, scope: !48)
!57 = !DILocation(line: 9, column: 20, scope: !58)
!58 = !DILexicalBlockFile(scope: !41, file: !1, discriminator: 2)
!59 = !DILocation(line: 9, column: 3, scope: !58)
!60 = distinct !{!60, !61, !62}
!61 = !DILocation(line: 9, column: 3, scope: !42)
!62 = !DILocation(line: 14, column: 30, scope: !42)
!63 = !DILocation(line: 15, column: 3, scope: !23)
