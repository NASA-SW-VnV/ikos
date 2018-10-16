; ModuleID = 'basic-loop.c.pp.bc'
source_filename = "basic-loop.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

@a = common global [10 x double] zeroinitializer, align 16, !dbg !0
; CHECK: define [10 x double]* @a, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @a, aggregate_zero, align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !14 {
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !21, metadata !22), !dbg !23
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !24, metadata !22), !dbg !25
  call void @llvm.dbg.value(metadata i32 0, i64 0, metadata !26, metadata !22), !dbg !27
  br label %3, !dbg !28

; <label>:3:                                      ; preds = %10, %2
  %.0 = phi i32 [ 0, %2 ], [ %11, %10 ]
  call void @llvm.dbg.value(metadata i32 %.0, i64 0, metadata !26, metadata !22), !dbg !27
  %4 = icmp slt i32 %.0, 10, !dbg !30
  br i1 %4, label %5, label %12, !dbg !33

; <label>:5:                                      ; preds = %3
  %6 = sitofp i32 %.0 to double, !dbg !35
  %7 = fmul double %6, 8.800000e-01, !dbg !37
  %8 = sext i32 %.0 to i64, !dbg !38
  %9 = getelementptr inbounds [10 x double], [10 x double]* @a, i64 0, i64 %8, !dbg !38
  store double %7, double* %9, align 8, !dbg !39
  br label %10, !dbg !40

; <label>:10:                                     ; preds = %5
  %11 = add nsw i32 %.0, 1, !dbg !41
  call void @llvm.dbg.value(metadata i32 %11, i64 0, metadata !26, metadata !22), !dbg !27
  br label %3, !dbg !43, !llvm.loop !44

; <label>:12:                                     ; preds = %3
  %13 = sitofp i32 %.0 to double, !dbg !47
  %14 = sext i32 %.0 to i64, !dbg !48
  %15 = getelementptr inbounds [10 x double], [10 x double]* @a, i64 0, i64 %14, !dbg !48
  store double %13, double* %15, align 8, !dbg !49
  ret i32 0, !dbg !50
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!10, !11, !12}
!llvm.ident = !{!13}

!0 = !DIGlobalVariableExpression(var: !1)
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "basic-loop.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/normal_optimization")
!4 = !{}
!5 = !{!0}
!6 = !DICompositeType(tag: DW_TAG_array_type, baseType: !7, size: 640, elements: !8)
!7 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!8 = !{!9}
!9 = !DISubrange(count: 10)
!10 = !{i32 2, !"Dwarf Version", i32 4}
!11 = !{i32 2, !"Debug Info Version", i32 3}
!12 = !{i32 1, !"PIC Level", i32 2}
!13 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!14 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 3, type: !15, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, unit: !2, variables: !4)
!15 = !DISubroutineType(types: !16)
!16 = !{!17, !17, !18}
!17 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!18 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !19, size: 64)
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !20, size: 64)
!20 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!21 = !DILocalVariable(name: "argc", arg: 1, scope: !14, file: !3, line: 3, type: !17)
!22 = !DIExpression()
!23 = !DILocation(line: 3, column: 14, scope: !14)
!24 = !DILocalVariable(name: "argv", arg: 2, scope: !14, file: !3, line: 3, type: !18)
!25 = !DILocation(line: 3, column: 27, scope: !14)
!26 = !DILocalVariable(name: "i", scope: !14, file: !3, line: 4, type: !17)
!27 = !DILocation(line: 4, column: 7, scope: !14)
!28 = !DILocation(line: 5, column: 8, scope: !29)
!29 = distinct !DILexicalBlock(scope: !14, file: !3, line: 5, column: 3)
!30 = !DILocation(line: 5, column: 17, scope: !31)
!31 = !DILexicalBlockFile(scope: !32, file: !3, discriminator: 1)
!32 = distinct !DILexicalBlock(scope: !29, file: !3, line: 5, column: 3)
!33 = !DILocation(line: 5, column: 3, scope: !34)
!34 = !DILexicalBlockFile(scope: !29, file: !3, discriminator: 1)
!35 = !DILocation(line: 6, column: 12, scope: !36)
!36 = distinct !DILexicalBlock(scope: !32, file: !3, line: 5, column: 28)
!37 = !DILocation(line: 6, column: 14, scope: !36)
!38 = !DILocation(line: 6, column: 5, scope: !36)
!39 = !DILocation(line: 6, column: 10, scope: !36)
!40 = !DILocation(line: 7, column: 3, scope: !36)
!41 = !DILocation(line: 5, column: 24, scope: !42)
!42 = !DILexicalBlockFile(scope: !32, file: !3, discriminator: 2)
!43 = !DILocation(line: 5, column: 3, scope: !42)
!44 = distinct !{!44, !45, !46}
!45 = !DILocation(line: 5, column: 3, scope: !29)
!46 = !DILocation(line: 7, column: 3, scope: !29)
!47 = !DILocation(line: 8, column: 10, scope: !14)
!48 = !DILocation(line: 8, column: 3, scope: !14)
!49 = !DILocation(line: 8, column: 8, scope: !14)
!50 = !DILocation(line: 9, column: 1, scope: !14)
