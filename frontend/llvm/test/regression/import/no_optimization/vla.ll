; ModuleID = 'vla.c.pp.bc'
source_filename = "vla.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.13.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.13.0

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
  %4 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !12, metadata !DIExpression()), !dbg !13
  %5 = load i32, i32* %2, align 4, !dbg !14
  %6 = zext i32 %5 to i64, !dbg !15
  %7 = call i8* @llvm.stacksave(), !dbg !15
  store i8* %7, i8** %3, align 8, !dbg !15
  %8 = alloca i32, i64 %6, align 16, !dbg !15
  call void @llvm.dbg.declare(metadata i32* %8, metadata !16, metadata !DIExpression()), !dbg !20
  call void @llvm.dbg.declare(metadata i32* %4, metadata !21, metadata !DIExpression()), !dbg !22
  store i32 0, i32* %4, align 4, !dbg !23
  br label %9, !dbg !25

; <label>:9:                                      ; preds = %20, %1
  %10 = load i32, i32* %4, align 4, !dbg !26
  %11 = load i32, i32* %2, align 4, !dbg !28
  %12 = icmp slt i32 %10, %11, !dbg !29
  br i1 %12, label %13, label %23, !dbg !30

; <label>:13:                                     ; preds = %9
  %14 = load i32, i32* %4, align 4, !dbg !31
  %15 = load i32, i32* %4, align 4, !dbg !33
  %16 = mul nsw i32 %14, %15, !dbg !34
  %17 = load i32, i32* %4, align 4, !dbg !35
  %18 = sext i32 %17 to i64, !dbg !36
  %19 = getelementptr inbounds i32, i32* %8, i64 %18, !dbg !36
  store i32 %16, i32* %19, align 4, !dbg !37
  br label %20, !dbg !38

; <label>:20:                                     ; preds = %13
  %21 = load i32, i32* %4, align 4, !dbg !39
  %22 = add nsw i32 %21, 1, !dbg !39
  store i32 %22, i32* %4, align 4, !dbg !39
  br label %9, !dbg !40, !llvm.loop !41

; <label>:23:                                     ; preds = %9
  %24 = load i32, i32* %2, align 4, !dbg !43
  %25 = load i32, i32* %2, align 4, !dbg !44
  %26 = mul nsw i32 %24, %25, !dbg !45
  %27 = load i32, i32* %2, align 4, !dbg !46
  %28 = sext i32 %27 to i64, !dbg !47
  %29 = getelementptr inbounds i32, i32* %8, i64 %28, !dbg !47
  store i32 %26, i32* %29, align 4, !dbg !48
  %30 = load i8*, i8** %3, align 8, !dbg !49
  call void @llvm.stackrestore(i8* %30), !dbg !49
  ret void, !dbg !49
}
; CHECK: define void @foo(si32 %1) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si32* $2 = allocate si32, 1, align 4
; CHECK:   si8** $3 = allocate si8*, 1, align 8
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   store $2, %1, align 4
; CHECK:   ui32* %5 = bitcast $2
; CHECK:   ui32 %6 = load %5, align 4
; CHECK:   ui64 %7 = zext %6
; CHECK:   si8* %8 = call @ar.stacksave()
; CHECK:   store $3, %8, align 8
; CHECK:   si32* $9 = allocate si32, %7, align 16
; CHECK:   store $4, 0, align 4
; CHECK: }
; CHECK: #2 predecessors={#1, #3} successors={#3, #4} {
; CHECK:   si32 %10 = load $4, align 4
; CHECK:   si32 %11 = load $2, align 4
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#2} {
; CHECK:   %10 silt %11
; CHECK:   si32 %12 = load $4, align 4
; CHECK:   si32 %13 = load $4, align 4
; CHECK:   si32 %14 = %12 smul.nw %13
; CHECK:   si32 %15 = load $4, align 4
; CHECK:   si64 %16 = sext %15
; CHECK:   si32* %17 = ptrshift $9, 4 * %16
; CHECK:   store %17, %14, align 4
; CHECK:   si32 %18 = load $4, align 4
; CHECK:   si32 %19 = %18 sadd.nw 1
; CHECK:   store $4, %19, align 4
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %10 sige %11
; CHECK:   si32 %20 = load $2, align 4
; CHECK:   si32 %21 = load $2, align 4
; CHECK:   si32 %22 = %20 smul.nw %21
; CHECK:   si32 %23 = load $2, align 4
; CHECK:   si64 %24 = sext %23
; CHECK:   si32* %25 = ptrshift $9, 4 * %24
; CHECK:   store %25, %22, align 4
; CHECK:   si8* %26 = load $3, align 8
; CHECK:   call @ar.stackrestore(%26)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !50 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !56, metadata !DIExpression()), !dbg !57
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !58, metadata !DIExpression()), !dbg !59
  call void @llvm.dbg.declare(metadata i32* %6, metadata !60, metadata !DIExpression()), !dbg !61
  %7 = getelementptr inbounds [3 x i8], [3 x i8]* @.str, i32 0, i32 0, !dbg !62
  %8 = call i32 (i8*, ...) @scanf(i8* %7, i32* %6), !dbg !62
  %9 = load i32, i32* %6, align 4, !dbg !63
  call void @foo(i32 %9), !dbg !64
  ret i32 0, !dbg !65
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

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 6.0.1 (tags/RELEASE_601/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "vla.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 6.0.1 (tags/RELEASE_601/final)"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 2, type: !9, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{null, !11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "n", arg: 1, scope: !8, file: !1, line: 2, type: !11)
!13 = !DILocation(line: 2, column: 14, scope: !8)
!14 = !DILocation(line: 3, column: 9, scope: !8)
!15 = !DILocation(line: 3, column: 3, scope: !8)
!16 = !DILocalVariable(name: "a", scope: !8, file: !1, line: 3, type: !17)
!17 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, elements: !18)
!18 = !{!19}
!19 = !DISubrange(count: -1)
!20 = !DILocation(line: 3, column: 7, scope: !8)
!21 = !DILocalVariable(name: "i", scope: !8, file: !1, line: 3, type: !11)
!22 = !DILocation(line: 3, column: 13, scope: !8)
!23 = !DILocation(line: 4, column: 10, scope: !24)
!24 = distinct !DILexicalBlock(scope: !8, file: !1, line: 4, column: 3)
!25 = !DILocation(line: 4, column: 8, scope: !24)
!26 = !DILocation(line: 4, column: 15, scope: !27)
!27 = distinct !DILexicalBlock(scope: !24, file: !1, line: 4, column: 3)
!28 = !DILocation(line: 4, column: 19, scope: !27)
!29 = !DILocation(line: 4, column: 17, scope: !27)
!30 = !DILocation(line: 4, column: 3, scope: !24)
!31 = !DILocation(line: 5, column: 12, scope: !32)
!32 = distinct !DILexicalBlock(scope: !27, file: !1, line: 4, column: 27)
!33 = !DILocation(line: 5, column: 16, scope: !32)
!34 = !DILocation(line: 5, column: 14, scope: !32)
!35 = !DILocation(line: 5, column: 7, scope: !32)
!36 = !DILocation(line: 5, column: 5, scope: !32)
!37 = !DILocation(line: 5, column: 10, scope: !32)
!38 = !DILocation(line: 6, column: 3, scope: !32)
!39 = !DILocation(line: 4, column: 23, scope: !27)
!40 = !DILocation(line: 4, column: 3, scope: !27)
!41 = distinct !{!41, !30, !42}
!42 = !DILocation(line: 6, column: 3, scope: !24)
!43 = !DILocation(line: 7, column: 10, scope: !8)
!44 = !DILocation(line: 7, column: 14, scope: !8)
!45 = !DILocation(line: 7, column: 12, scope: !8)
!46 = !DILocation(line: 7, column: 5, scope: !8)
!47 = !DILocation(line: 7, column: 3, scope: !8)
!48 = !DILocation(line: 7, column: 8, scope: !8)
!49 = !DILocation(line: 8, column: 1, scope: !8)
!50 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !51, isLocal: false, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!51 = !DISubroutineType(types: !52)
!52 = !{!11, !11, !53}
!53 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !54, size: 64)
!54 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !55, size: 64)
!55 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!56 = !DILocalVariable(name: "argc", arg: 1, scope: !50, file: !1, line: 10, type: !11)
!57 = !DILocation(line: 10, column: 14, scope: !50)
!58 = !DILocalVariable(name: "argv", arg: 2, scope: !50, file: !1, line: 10, type: !53)
!59 = !DILocation(line: 10, column: 27, scope: !50)
!60 = !DILocalVariable(name: "v", scope: !50, file: !1, line: 11, type: !11)
!61 = !DILocation(line: 11, column: 7, scope: !50)
!62 = !DILocation(line: 12, column: 3, scope: !50)
!63 = !DILocation(line: 13, column: 7, scope: !50)
!64 = !DILocation(line: 13, column: 3, scope: !50)
!65 = !DILocation(line: 14, column: 3, scope: !50)
