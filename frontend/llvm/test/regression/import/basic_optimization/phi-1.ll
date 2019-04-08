; ModuleID = 'phi-1.pp.bc'
source_filename = "phi-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

@a = common global [10 x double] zeroinitializer, align 16, !dbg !0
; CHECK: define [10 x double]* @a, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @a, aggregate_zero, align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !15 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !22, metadata !DIExpression()), !dbg !23
  call void @llvm.dbg.value(metadata i8** %1, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.value(metadata i32 0, metadata !26, metadata !DIExpression()), !dbg !27
  br label %3, !dbg !28

; <label>:3:                                      ; preds = %10, %2
  %.0 = phi i32 [ 0, %2 ], [ %11, %10 ], !dbg !30
  call void @llvm.dbg.value(metadata i32 %.0, metadata !26, metadata !DIExpression()), !dbg !27
  %4 = icmp slt i32 %.0, 10, !dbg !31
  br i1 %4, label %5, label %12, !dbg !33

; <label>:5:                                      ; preds = %3
  %6 = sitofp i32 %.0 to double, !dbg !34
  %7 = fmul double %6, 8.800000e-01, !dbg !36
  %8 = sext i32 %.0 to i64, !dbg !37
  %9 = getelementptr inbounds [10 x double], [10 x double]* @a, i64 0, i64 %8, !dbg !37
  store double %7, double* %9, align 8, !dbg !38
  br label %10, !dbg !39

; <label>:10:                                     ; preds = %5
  %11 = add nsw i32 %.0, 1, !dbg !40
  call void @llvm.dbg.value(metadata i32 %11, metadata !26, metadata !DIExpression()), !dbg !27
  br label %3, !dbg !41, !llvm.loop !42

; <label>:12:                                     ; preds = %3
  %13 = sitofp i32 %.0 to double, !dbg !44
  %14 = sext i32 %.0 to i64, !dbg !45
  %15 = getelementptr inbounds [10 x double], [10 x double]* @a, i64 0, i64 %14, !dbg !45
  store double %13, double* %15, align 8, !dbg !46
  ret i32 0, !dbg !47
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32 %.0 = 0
; CHECK: }
; CHECK: #2 predecessors={#1, #3} successors={#3, #4} {
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#2} {
; CHECK:   %.0 silt 10
; CHECK:   double %3 = sitofp %.0
; CHECK:   double %4 = %3 fmul 8.8E-1
; CHECK:   si64 %5 = sext %.0
; CHECK:   double* %6 = ptrshift @a, 80 * 0, 8 * %5
; CHECK:   store %6, %4, align 8
; CHECK:   si32 %7 = %.0 sadd.nw 1
; CHECK:   si32 %.0 = %7
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %.0 sige 10
; CHECK:   double %8 = sitofp %.0
; CHECK:   si64 %9 = sext %.0
; CHECK:   double* %10 = ptrshift @a, 80 * 0, 8 * %9
; CHECK:   store %10, %8, align 8
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!10, !11, !12, !13}
!llvm.ident = !{!14}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 8.0.0 (tags/RELEASE_800/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "phi-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!4 = !{}
!5 = !{!0}
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 640, elements: !8)
!7 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!8 = !{!9}
!9 = !DISubrange(count: 10)
!10 = !{i32 2, !"Dwarf Version", i32 4}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 1, !"wchar_size", i32 4}
!13 = !{i32 7, !"PIC Level", i32 2}
!14 = !{!"clang version 8.0.0 (tags/RELEASE_800/final)"}
!15 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 3, type: !16, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!16 = !DISubroutineType(types: !17)
!17 = !{!18, !18, !19}
!18 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!20 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !21, size: 64)
!21 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!22 = !DILocalVariable(name: "argc", arg: 1, scope: !15, file: !3, line: 3, type: !18)
!23 = !DILocation(line: 3, column: 14, scope: !15)
!24 = !DILocalVariable(name: "argv", arg: 2, scope: !15, file: !3, line: 3, type: !19)
!25 = !DILocation(line: 3, column: 27, scope: !15)
!26 = !DILocalVariable(name: "i", scope: !15, file: !3, line: 4, type: !18)
!27 = !DILocation(line: 4, column: 7, scope: !15)
!28 = !DILocation(line: 5, column: 8, scope: !29)
!29 = distinct !DILexicalBlock(scope: !15, file: !3, line: 5, column: 3)
!30 = !DILocation(line: 0, scope: !29)
!31 = !DILocation(line: 5, column: 17, scope: !32)
!32 = distinct !DILexicalBlock(scope: !29, file: !3, line: 5, column: 3)
!33 = !DILocation(line: 5, column: 3, scope: !29)
!34 = !DILocation(line: 6, column: 12, scope: !35)
!35 = distinct !DILexicalBlock(scope: !32, file: !3, line: 5, column: 28)
!36 = !DILocation(line: 6, column: 14, scope: !35)
!37 = !DILocation(line: 6, column: 5, scope: !35)
!38 = !DILocation(line: 6, column: 10, scope: !35)
!39 = !DILocation(line: 7, column: 3, scope: !35)
!40 = !DILocation(line: 5, column: 24, scope: !32)
!41 = !DILocation(line: 5, column: 3, scope: !32)
!42 = distinct !{!42, !33, !43}
!43 = !DILocation(line: 7, column: 3, scope: !29)
!44 = !DILocation(line: 8, column: 10, scope: !15)
!45 = !DILocation(line: 8, column: 3, scope: !15)
!46 = !DILocation(line: 8, column: 8, scope: !15)
!47 = !DILocation(line: 9, column: 1, scope: !15)
