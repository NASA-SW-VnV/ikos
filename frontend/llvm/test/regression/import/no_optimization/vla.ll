; ModuleID = 'vla.pp.bc'
source_filename = "vla.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
; CHECK: define [3 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [37, 100, 0], align 1
; CHECK: }
; CHECK: }

declare i32 @scanf(i8*, ...) #3
; CHECK: declare si32 @ar.libc.scanf(si8*, ...)

; Function Attrs: nounwind
declare void @llvm.stackrestore(i8*) #2
; CHECK: declare void @ar.stackrestore(si8*)

; Function Attrs: nounwind
declare i8* @llvm.stacksave() #2
; CHECK: declare si8* @ar.stacksave()

; Function Attrs: noinline nounwind ssp uwtable
define void @foo(i32) #0 !dbg !8 {
  %2 = alloca i32, align 4
  %3 = alloca i8*, align 8
  %4 = alloca i64, align 8
  %5 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !12, metadata !DIExpression()), !dbg !13
  %6 = load i32, i32* %2, align 4, !dbg !14
  %7 = zext i32 %6 to i64, !dbg !15
  %8 = call i8* @llvm.stacksave(), !dbg !15
  store i8* %8, i8** %3, align 8, !dbg !15
  %9 = alloca i32, i64 %7, align 16, !dbg !15
  store i64 %7, i64* %4, align 8, !dbg !15
  call void @llvm.dbg.declare(metadata i64* %4, metadata !16, metadata !DIExpression()), !dbg !18
  call void @llvm.dbg.declare(metadata i32* %9, metadata !19, metadata !DIExpression()), !dbg !23
  call void @llvm.dbg.declare(metadata i32* %5, metadata !24, metadata !DIExpression()), !dbg !25
  store i32 0, i32* %5, align 4, !dbg !26
  br label %10, !dbg !28

10:                                               ; preds = %21, %1
  %11 = load i32, i32* %5, align 4, !dbg !29
  %12 = load i32, i32* %2, align 4, !dbg !31
  %13 = icmp slt i32 %11, %12, !dbg !32
  br i1 %13, label %14, label %24, !dbg !33

14:                                               ; preds = %10
  %15 = load i32, i32* %5, align 4, !dbg !34
  %16 = load i32, i32* %5, align 4, !dbg !36
  %17 = mul nsw i32 %15, %16, !dbg !37
  %18 = load i32, i32* %5, align 4, !dbg !38
  %19 = sext i32 %18 to i64, !dbg !39
  %20 = getelementptr inbounds i32, i32* %9, i64 %19, !dbg !39
  store i32 %17, i32* %20, align 4, !dbg !40
  br label %21, !dbg !41

21:                                               ; preds = %14
  %22 = load i32, i32* %5, align 4, !dbg !42
  %23 = add nsw i32 %22, 1, !dbg !42
  store i32 %23, i32* %5, align 4, !dbg !42
  br label %10, !dbg !43, !llvm.loop !44

24:                                               ; preds = %10
  %25 = load i32, i32* %2, align 4, !dbg !46
  %26 = load i32, i32* %2, align 4, !dbg !47
  %27 = mul nsw i32 %25, %26, !dbg !48
  %28 = load i32, i32* %2, align 4, !dbg !49
  %29 = sext i32 %28 to i64, !dbg !50
  %30 = getelementptr inbounds i32, i32* %9, i64 %29, !dbg !50
  store i32 %27, i32* %30, align 4, !dbg !51
  %31 = load i8*, i8** %3, align 8, !dbg !52
  call void @llvm.stackrestore(i8* %31), !dbg !52
  ret void, !dbg !52
}
; CHECK: define void @foo(si32 %1) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32* $2 = allocate si32, 1, align 4
; CHECK:   si8** $3 = allocate si8*, 1, align 8
; CHECK:   ui64* $4 = allocate ui64, 1, align 8
; CHECK:   si32* $5 = allocate si32, 1, align 4
; CHECK:   store $2, %1, align 4
; CHECK:   ui32* %6 = bitcast $2
; CHECK:   ui32 %7 = load %6, align 4
; CHECK:   ui64 %8 = zext %7
; CHECK:   si8* %9 = call @ar.stacksave()
; CHECK:   store $3, %9, align 8
; CHECK:   si32* $10 = allocate si32, %8, align 16
; CHECK:   store $4, %8, align 8
; CHECK:   store $5, 0, align 4
; CHECK: }
; CHECK: #2 predecessors={#1, #3} successors={#3, #4} {
; CHECK:   si32 %11 = load $5, align 4
; CHECK:   si32 %12 = load $2, align 4
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#2} {
; CHECK:   %11 silt %12
; CHECK:   si32 %13 = load $5, align 4
; CHECK:   si32 %14 = load $5, align 4
; CHECK:   si32 %15 = %13 smul.nw %14
; CHECK:   si32 %16 = load $5, align 4
; CHECK:   si64 %17 = sext %16
; CHECK:   si32* %18 = ptrshift $10, 4 * %17
; CHECK:   store %18, %15, align 4
; CHECK:   si32 %19 = load $5, align 4
; CHECK:   si32 %20 = %19 sadd.nw 1
; CHECK:   store $5, %20, align 4
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %11 sige %12
; CHECK:   si32 %21 = load $2, align 4
; CHECK:   si32 %22 = load $2, align 4
; CHECK:   si32 %23 = %21 smul.nw %22
; CHECK:   si32 %24 = load $2, align 4
; CHECK:   si64 %25 = sext %24
; CHECK:   si32* %26 = ptrshift $10, 4 * %25
; CHECK:   store %26, %23, align 4
; CHECK:   si8* %27 = load $3, align 8
; CHECK:   call @ar.stackrestore(%27)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !53 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !59, metadata !DIExpression()), !dbg !60
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !61, metadata !DIExpression()), !dbg !62
  call void @llvm.dbg.declare(metadata i32* %6, metadata !63, metadata !DIExpression()), !dbg !64
  %7 = getelementptr inbounds [3 x i8], [3 x i8]* @.str, i64 0, i64 0, !dbg !65
  %8 = call i32 (i8*, ...) @scanf(i8* %7, i32* %6), !dbg !65
  %9 = load i32, i32* %6, align 4, !dbg !66
  call void @foo(i32 %9), !dbg !67
  ret i32 0, !dbg !68
}
; CHECK: define si32 @main(si32 %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si8*** $5 = allocate si8**, 1, align 8
; CHECK:   si32* $6 = allocate si32, 1, align 4
; CHECK:   store $3, 0, align 4
; CHECK:   store $4, %1, align 4
; CHECK:   store $5, %2, align 8
; CHECK:   si8* %7 = ptrshift @.str, 3 * 0, 1 * 0
; CHECK:   si32 %8 = call @ar.libc.scanf(%7, $6)
; CHECK:   si32 %9 = load $6, align 4
; CHECK:   call @foo(%9)
; CHECK:   return 0
; CHECK: }
; CHECK: }

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "vla.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 2, type: !9, scopeLine: 2, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "n", arg: 1, scope: !8, file: !1, line: 2, type: !11)
!13 = !DILocation(line: 2, column: 14, scope: !8)
!14 = !DILocation(line: 3, column: 9, scope: !8)
!15 = !DILocation(line: 3, column: 3, scope: !8)
!16 = !DILocalVariable(name: "__vla_expr0", scope: !8, type: !17, flags: DIFlagArtificial)
!17 = !DIBasicType(name: "long unsigned int", size: 64, encoding: DW_ATE_unsigned)
!18 = !DILocation(line: 0, scope: !8)
!19 = !DILocalVariable(name: "a", scope: !8, file: !1, line: 3, type: !20)
!20 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, elements: !21)
!21 = !{!22}
!22 = !DISubrange(count: !16)
!23 = !DILocation(line: 3, column: 7, scope: !8)
!24 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 3, type: !11)
!25 = !DILocation(line: 3, column: 13, scope: !8)
!26 = !DILocation(line: 4, column: 10, scope: !27)
!27 = distinct !DILexicalBlock(scope: !8, file: !1, line: 4, column: 3)
!28 = !DILocation(line: 4, column: 8, scope: !27)
!29 = !DILocation(line: 4, column: 15, scope: !30)
!30 = distinct !DILexicalBlock(scope: !27, file: !1, line: 4, column: 3)
!31 = !DILocation(line: 4, column: 19, scope: !30)
!32 = !DILocation(line: 4, column: 17, scope: !30)
!33 = !DILocation(line: 4, column: 3, scope: !27)
!34 = !DILocation(line: 5, column: 12, scope: !35)
!35 = distinct !DILexicalBlock(scope: !30, file: !1, line: 4, column: 27)
!36 = !DILocation(line: 5, column: 16, scope: !35)
!37 = !DILocation(line: 5, column: 14, scope: !35)
!38 = !DILocation(line: 5, column: 7, scope: !35)
!39 = !DILocation(line: 5, column: 5, scope: !35)
!40 = !DILocation(line: 5, column: 10, scope: !35)
!41 = !DILocation(line: 6, column: 3, scope: !35)
!42 = !DILocation(line: 4, column: 23, scope: !30)
!43 = !DILocation(line: 4, column: 3, scope: !30)
!44 = distinct !{!44, !33, !45}
!45 = !DILocation(line: 6, column: 3, scope: !27)
!46 = !DILocation(line: 7, column: 10, scope: !8)
!47 = !DILocation(line: 7, column: 14, scope: !8)
!48 = !DILocation(line: 7, column: 12, scope: !8)
!49 = !DILocation(line: 7, column: 5, scope: !8)
!50 = !DILocation(line: 7, column: 3, scope: !8)
!51 = !DILocation(line: 7, column: 8, scope: !8)
!52 = !DILocation(line: 8, column: 1, scope: !8)
!53 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !54, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!54 = !DISubroutineType(types: !55)
!55 = !{!11, !11, !56}
!56 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !57, size: 64)
!57 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !58, size: 64)
!58 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!59 = !DILocalVariable(name: "argc", arg: 1, scope: !53, file: !1, line: 10, type: !11)
!60 = !DILocation(line: 10, column: 14, scope: !53)
!61 = !DILocalVariable(name: "argv", arg: 2, scope: !53, file: !1, line: 10, type: !56)
!62 = !DILocation(line: 10, column: 27, scope: !53)
!63 = !DILocalVariable(name: "v", scope: !53, file: !1, line: 11, type: !11)
!64 = !DILocation(line: 11, column: 7, scope: !53)
!65 = !DILocation(line: 12, column: 3, scope: !53)
!66 = !DILocation(line: 13, column: 7, scope: !53)
!67 = !DILocation(line: 13, column: 3, scope: !53)
!68 = !DILocation(line: 14, column: 3, scope: !53)
