; ModuleID = 'basic-loop.pp.bc'
source_filename = "basic-loop.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

@a = common global [10 x double] zeroinitializer, align 16, !dbg !0
; CHECK: define [10 x double]* @a, align 16, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @a, aggregate_zero, align 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !15 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !22, metadata !DIExpression()), !dbg !23
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !24, metadata !DIExpression()), !dbg !25
  call void @llvm.dbg.declare(metadata i32* %6, metadata !26, metadata !DIExpression()), !dbg !27
  store i32 0, i32* %6, align 4, !dbg !28
  br label %7, !dbg !30

7:                                                ; preds = %17, %2
  %8 = load i32, i32* %6, align 4, !dbg !31
  %9 = icmp slt i32 %8, 10, !dbg !33
  br i1 %9, label %10, label %20, !dbg !34

10:                                               ; preds = %7
  %11 = load i32, i32* %6, align 4, !dbg !35
  %12 = sitofp i32 %11 to double, !dbg !35
  %13 = fmul double %12, 8.800000e-01, !dbg !37
  %14 = load i32, i32* %6, align 4, !dbg !38
  %15 = sext i32 %14 to i64, !dbg !39
  %16 = getelementptr inbounds [10 x double], [10 x double]* @a, i64 0, i64 %15, !dbg !39
  store double %13, double* %16, align 8, !dbg !40
  br label %17, !dbg !41

17:                                               ; preds = %10
  %18 = load i32, i32* %6, align 4, !dbg !42
  %19 = add nsw i32 %18, 1, !dbg !42
  store i32 %19, i32* %6, align 4, !dbg !42
  br label %7, !dbg !43, !llvm.loop !44

20:                                               ; preds = %7
  %21 = load i32, i32* %6, align 4, !dbg !46
  %22 = sitofp i32 %21 to double, !dbg !46
  %23 = load i32, i32* %6, align 4, !dbg !47
  %24 = sext i32 %23 to i64, !dbg !48
  %25 = getelementptr inbounds [10 x double], [10 x double]* @a, i64 0, i64 %24, !dbg !48
  store double %22, double* %25, align 8, !dbg !49
  %26 = load i32, i32* %3, align 4, !dbg !50
  ret i32 %26, !dbg !50
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si8*** $5 = allocate si8**, 1, align 8
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   store $3, 0, align 4
; CHECK:   store $4, %1, align 4
; CHECK:   store $5, %2, align 8
; CHECK:   store $6, 0, align 4
; CHECK: }
; CHECK: #2 predecessors={#1, #3} successors={#3, #4} {
; CHECK:   si32 %7 = load $6, align 4
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#2} {
; CHECK:   %7 silt 10
; CHECK:   si32 %8 = load $6, align 4
; CHECK:   double %9 = sitofp %8
; CHECK:   double %10 = %9 fmul 8.8E-1
; CHECK:   si32 %11 = load $6, align 4
; CHECK:   si64 %12 = sext %11
; CHECK:   double* %13 = ptrshift @a, 80 * 0, 8 * %12
; CHECK:   store %13, %10, align 8
; CHECK:   si32 %14 = load $6, align 4
; CHECK:   si32 %15 = %14 sadd.nw 1
; CHECK:   store $6, %15, align 4
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %7 sige 10
; CHECK:   si32 %16 = load $6, align 4
; CHECK:   double %17 = sitofp %16
; CHECK:   si32 %18 = load $6, align 4
; CHECK:   si64 %19 = sext %18
; CHECK:   double* %20 = ptrshift @a, 80 * 0, 8 * %19
; CHECK:   store %20, %17, align 8
; CHECK:   si32 %21 = load $3, align 4
; CHECK:   return %21
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!10, !11, !12, !13}
!llvm.ident = !{!14}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "basic-loop.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!14 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
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
!28 = !DILocation(line: 5, column: 10, scope: !29)
!29 = distinct !DILexicalBlock(scope: !15, file: !3, line: 5, column: 3)
!30 = !DILocation(line: 5, column: 8, scope: !29)
!31 = !DILocation(line: 5, column: 15, scope: !32)
!32 = distinct !DILexicalBlock(scope: !29, file: !3, line: 5, column: 3)
!33 = !DILocation(line: 5, column: 17, scope: !32)
!34 = !DILocation(line: 5, column: 3, scope: !29)
!35 = !DILocation(line: 6, column: 12, scope: !36)
!36 = distinct !DILexicalBlock(scope: !32, file: !3, line: 5, column: 28)
!37 = !DILocation(line: 6, column: 14, scope: !36)
!38 = !DILocation(line: 6, column: 7, scope: !36)
!39 = !DILocation(line: 6, column: 5, scope: !36)
!40 = !DILocation(line: 6, column: 10, scope: !36)
!41 = !DILocation(line: 7, column: 3, scope: !36)
!42 = !DILocation(line: 5, column: 24, scope: !32)
!43 = !DILocation(line: 5, column: 3, scope: !32)
!44 = distinct !{!44, !34, !45}
!45 = !DILocation(line: 7, column: 3, scope: !29)
!46 = !DILocation(line: 8, column: 10, scope: !15)
!47 = !DILocation(line: 8, column: 5, scope: !15)
!48 = !DILocation(line: 8, column: 3, scope: !15)
!49 = !DILocation(line: 8, column: 8, scope: !15)
!50 = !DILocation(line: 9, column: 1, scope: !15)
