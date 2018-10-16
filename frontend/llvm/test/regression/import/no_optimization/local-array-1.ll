; ModuleID = 'local-array-1.c.pp.bc'
source_filename = "local-array-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1
; CHECK: define [4 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [37, 100, 10, 0], align 1
; CHECK: }
; CHECK: }

declare i32 @printf(i8*, ...) #2
; CHECK: declare si32 @ar.libc.printf(si8*, ...)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !7 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  %7 = alloca [10 x i32], align 16
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !14, metadata !15), !dbg !16
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !17, metadata !15), !dbg !18
  call void @llvm.dbg.declare(metadata i32* %6, metadata !19, metadata !15), !dbg !20
  call void @llvm.dbg.declare(metadata [10 x i32]* %7, metadata !21, metadata !15), !dbg !25
  store i32 0, i32* %6, align 4, !dbg !26
  br label %8, !dbg !28

; <label>:8:                                      ; preds = %16, %2
  %9 = load i32, i32* %6, align 4, !dbg !29
  %10 = icmp slt i32 %9, 10, !dbg !32
  br i1 %10, label %11, label %19, !dbg !33

; <label>:11:                                     ; preds = %8
  %12 = load i32, i32* %6, align 4, !dbg !35
  %13 = load i32, i32* %6, align 4, !dbg !37
  %14 = sext i32 %13 to i64, !dbg !38
  %15 = getelementptr inbounds [10 x i32], [10 x i32]* %7, i64 0, i64 %14, !dbg !38
  store i32 %12, i32* %15, align 4, !dbg !39
  br label %16, !dbg !40

; <label>:16:                                     ; preds = %11
  %17 = load i32, i32* %6, align 4, !dbg !41
  %18 = add nsw i32 %17, 1, !dbg !41
  store i32 %18, i32* %6, align 4, !dbg !41
  br label %8, !dbg !43, !llvm.loop !44

; <label>:19:                                     ; preds = %8
  %20 = load i32, i32* %6, align 4, !dbg !47
  %21 = sub nsw i32 %20, 1, !dbg !48
  %22 = sext i32 %21 to i64, !dbg !49
  %23 = getelementptr inbounds [10 x i32], [10 x i32]* %7, i64 0, i64 %22, !dbg !49
  %24 = load i32, i32* %23, align 4, !dbg !49
  %25 = getelementptr inbounds [4 x i8], [4 x i8]* @.str, i32 0, i32 0, !dbg !50
  %26 = call i32 (i8*, ...) @printf(i8* %25, i32 %24), !dbg !50
  ret i32 0, !dbg !51
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si8*** $5 = allocate si8**, 1, align 8
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   [10 x si32]* $7 = allocate [10 x si32], 1, align 16
; CHECK:   store $3, 0, align 4
; CHECK:   store $4, %1, align 4
; CHECK:   store $5, %2, align 8
; CHECK:   store $6, 0, align 4
; CHECK: }
; CHECK: #2 predecessors={#1, #3} successors={#3, #4} {
; CHECK:   si32 %8 = load $6, align 4
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#2} {
; CHECK:   %8 silt 10
; CHECK:   si32 %9 = load $6, align 4
; CHECK:   si32 %10 = load $6, align 4
; CHECK:   si64 %11 = sext %10
; CHECK:   si32* %12 = ptrshift $7, 40 * 0, 4 * %11
; CHECK:   store %12, %9, align 4
; CHECK:   si32 %13 = load $6, align 4
; CHECK:   si32 %14 = %13 sadd.nw 1
; CHECK:   store $6, %14, align 4
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %8 sige 10
; CHECK:   si32 %15 = load $6, align 4
; CHECK:   si32 %16 = %15 ssub.nw 1
; CHECK:   si64 %17 = sext %16
; CHECK:   si32* %18 = ptrshift $7, 40 * 0, 4 * %17
; CHECK:   si32 %19 = load %18, align 4
; CHECK:   si8* %20 = ptrshift @.str, 4 * 0, 1 * 0
; CHECK:   si32 %21 = call @ar.libc.printf(%20, %19)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "local-array-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 6, type: !8, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!14 = !DILocalVariable(name: "argc", arg: 1, scope: !7, file: !1, line: 6, type: !10)
!15 = !DIExpression()
!16 = !DILocation(line: 6, column: 14, scope: !7)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !7, file: !1, line: 6, type: !11)
!18 = !DILocation(line: 6, column: 27, scope: !7)
!19 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 7, type: !10)
!20 = !DILocation(line: 7, column: 7, scope: !7)
!21 = !DILocalVariable(name: "a", scope: !7, file: !1, line: 8, type: !22)
!22 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, size: 320, elements: !23)
!23 = !{!24}
!24 = !DISubrange(count: 10)
!25 = !DILocation(line: 8, column: 7, scope: !7)
!26 = !DILocation(line: 9, column: 10, scope: !27)
!27 = distinct !DILexicalBlock(scope: !7, file: !1, line: 9, column: 3)
!28 = !DILocation(line: 9, column: 8, scope: !27)
!29 = !DILocation(line: 9, column: 15, scope: !30)
!30 = !DILexicalBlockFile(scope: !31, file: !1, discriminator: 1)
!31 = distinct !DILexicalBlock(scope: !27, file: !1, line: 9, column: 3)
!32 = !DILocation(line: 9, column: 17, scope: !30)
!33 = !DILocation(line: 9, column: 3, scope: !34)
!34 = !DILexicalBlockFile(scope: !27, file: !1, discriminator: 1)
!35 = !DILocation(line: 10, column: 12, scope: !36)
!36 = distinct !DILexicalBlock(scope: !31, file: !1, line: 9, column: 28)
!37 = !DILocation(line: 10, column: 7, scope: !36)
!38 = !DILocation(line: 10, column: 5, scope: !36)
!39 = !DILocation(line: 10, column: 10, scope: !36)
!40 = !DILocation(line: 11, column: 3, scope: !36)
!41 = !DILocation(line: 9, column: 24, scope: !42)
!42 = !DILexicalBlockFile(scope: !31, file: !1, discriminator: 2)
!43 = !DILocation(line: 9, column: 3, scope: !42)
!44 = distinct !{!44, !45, !46}
!45 = !DILocation(line: 9, column: 3, scope: !27)
!46 = !DILocation(line: 11, column: 3, scope: !27)
!47 = !DILocation(line: 12, column: 20, scope: !7)
!48 = !DILocation(line: 12, column: 22, scope: !7)
!49 = !DILocation(line: 12, column: 18, scope: !7)
!50 = !DILocation(line: 12, column: 3, scope: !7)
!51 = !DILocation(line: 13, column: 3, scope: !7)
