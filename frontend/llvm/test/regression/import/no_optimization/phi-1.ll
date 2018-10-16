; ModuleID = 'phi-1.c.pp.bc'
source_filename = "phi-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEl: Bundle
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
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !21, metadata !22), !dbg !23
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !24, metadata !22), !dbg !25
  call void @llvm.dbg.declare(metadata i32* %6, metadata !26, metadata !22), !dbg !27
  store i32 0, i32* %6, align 4, !dbg !28
  br label %7, !dbg !30

; <label>:7:                                      ; preds = %17, %2
  %8 = load i32, i32* %6, align 4, !dbg !31
  %9 = icmp slt i32 %8, 10, !dbg !34
  br i1 %9, label %10, label %20, !dbg !35

; <label>:10:                                     ; preds = %7
  %11 = load i32, i32* %6, align 4, !dbg !37
  %12 = sitofp i32 %11 to double, !dbg !37
  %13 = fmul double %12, 8.800000e-01, !dbg !39
  %14 = load i32, i32* %6, align 4, !dbg !40
  %15 = sext i32 %14 to i64, !dbg !41
  %16 = getelementptr inbounds [10 x double], [10 x double]* @a, i64 0, i64 %15, !dbg !41
  store double %13, double* %16, align 8, !dbg !42
  br label %17, !dbg !43

; <label>:17:                                     ; preds = %10
  %18 = load i32, i32* %6, align 4, !dbg !44
  %19 = add nsw i32 %18, 1, !dbg !44
  store i32 %19, i32* %6, align 4, !dbg !44
  br label %7, !dbg !46, !llvm.loop !47

; <label>:20:                                     ; preds = %7
  %21 = load i32, i32* %6, align 4, !dbg !50
  %22 = sitofp i32 %21 to double, !dbg !50
  %23 = load i32, i32* %6, align 4, !dbg !51
  %24 = sext i32 %23 to i64, !dbg !52
  %25 = getelementptr inbounds [10 x double], [10 x double]* @a, i64 0, i64 %24, !dbg !52
  store double %22, double* %25, align 8, !dbg !53
  %26 = load i32, i32* %3, align 4, !dbg !54
  ret i32 %26, !dbg !54
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!10, !11, !12}
!llvm.ident = !{!13}

!0 = !DIGlobalVariableExpression(var: !1)
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "phi-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
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
!28 = !DILocation(line: 5, column: 10, scope: !29)
!29 = distinct !DILexicalBlock(scope: !14, file: !3, line: 5, column: 3)
!30 = !DILocation(line: 5, column: 8, scope: !29)
!31 = !DILocation(line: 5, column: 15, scope: !32)
!32 = !DILexicalBlockFile(scope: !33, file: !3, discriminator: 1)
!33 = distinct !DILexicalBlock(scope: !29, file: !3, line: 5, column: 3)
!34 = !DILocation(line: 5, column: 17, scope: !32)
!35 = !DILocation(line: 5, column: 3, scope: !36)
!36 = !DILexicalBlockFile(scope: !29, file: !3, discriminator: 1)
!37 = !DILocation(line: 6, column: 12, scope: !38)
!38 = distinct !DILexicalBlock(scope: !33, file: !3, line: 5, column: 28)
!39 = !DILocation(line: 6, column: 14, scope: !38)
!40 = !DILocation(line: 6, column: 7, scope: !38)
!41 = !DILocation(line: 6, column: 5, scope: !38)
!42 = !DILocation(line: 6, column: 10, scope: !38)
!43 = !DILocation(line: 7, column: 3, scope: !38)
!44 = !DILocation(line: 5, column: 24, scope: !45)
!45 = !DILexicalBlockFile(scope: !33, file: !3, discriminator: 2)
!46 = !DILocation(line: 5, column: 3, scope: !45)
!47 = distinct !{!47, !48, !49}
!48 = !DILocation(line: 5, column: 3, scope: !29)
!49 = !DILocation(line: 7, column: 3, scope: !29)
!50 = !DILocation(line: 8, column: 10, scope: !14)
!51 = !DILocation(line: 8, column: 5, scope: !14)
!52 = !DILocation(line: 8, column: 3, scope: !14)
!53 = !DILocation(line: 8, column: 8, scope: !14)
!54 = !DILocation(line: 9, column: 1, scope: !14)
