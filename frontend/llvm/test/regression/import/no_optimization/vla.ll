; ModuleID = 'vla.c.pp.bc'
source_filename = "vla.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK: // Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

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
define void @foo(i32) #0 !dbg !7 {
  %2 = alloca i32, align 4
  %3 = alloca i8*, align 8
  %4 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !11, metadata !12), !dbg !13
  %5 = load i32, i32* %2, align 4, !dbg !14
  %6 = zext i32 %5 to i64, !dbg !15
  %7 = call i8* @llvm.stacksave(), !dbg !15
  store i8* %7, i8** %3, align 8, !dbg !15
  %8 = alloca i32, i64 %6, align 16, !dbg !15
  call void @llvm.dbg.declare(metadata i32* %8, metadata !16, metadata !20), !dbg !21
  call void @llvm.dbg.declare(metadata i32* %4, metadata !22, metadata !12), !dbg !23
  store i32 0, i32* %4, align 4, !dbg !24
  br label %9, !dbg !26

; <label>:9:                                      ; preds = %20, %1
  %10 = load i32, i32* %4, align 4, !dbg !27
  %11 = load i32, i32* %2, align 4, !dbg !30
  %12 = icmp slt i32 %10, %11, !dbg !31
  br i1 %12, label %13, label %23, !dbg !32

; <label>:13:                                     ; preds = %9
  %14 = load i32, i32* %4, align 4, !dbg !34
  %15 = load i32, i32* %4, align 4, !dbg !36
  %16 = mul nsw i32 %14, %15, !dbg !37
  %17 = load i32, i32* %4, align 4, !dbg !38
  %18 = sext i32 %17 to i64, !dbg !39
  %19 = getelementptr inbounds i32, i32* %8, i64 %18, !dbg !39
  store i32 %16, i32* %19, align 4, !dbg !40
  br label %20, !dbg !41

; <label>:20:                                     ; preds = %13
  %21 = load i32, i32* %4, align 4, !dbg !42
  %22 = add nsw i32 %21, 1, !dbg !42
  store i32 %22, i32* %4, align 4, !dbg !42
  br label %9, !dbg !44, !llvm.loop !45

; <label>:23:                                     ; preds = %9
  %24 = load i32, i32* %2, align 4, !dbg !48
  %25 = load i32, i32* %2, align 4, !dbg !49
  %26 = mul nsw i32 %24, %25, !dbg !50
  %27 = load i32, i32* %2, align 4, !dbg !51
  %28 = sext i32 %27 to i64, !dbg !52
  %29 = getelementptr inbounds i32, i32* %8, i64 %28, !dbg !52
  store i32 %26, i32* %29, align 4, !dbg !53
  %30 = load i8*, i8** %3, align 8, !dbg !54
  call void @llvm.stackrestore(i8* %30), !dbg !54
  ret void, !dbg !54
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !55 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !61, metadata !12), !dbg !62
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !63, metadata !12), !dbg !64
  call void @llvm.dbg.declare(metadata i32* %6, metadata !65, metadata !12), !dbg !66
  %7 = getelementptr inbounds [3 x i8], [3 x i8]* @.str, i32 0, i32 0, !dbg !67
  %8 = call i32 (i8*, ...) @scanf(i8* %7, i32* %6), !dbg !67
  %9 = load i32, i32* %6, align 4, !dbg !68
  call void @foo(i32 %9), !dbg !69
  ret i32 0, !dbg !70
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

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "vla.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 2, type: !8, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{null, !10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "n", arg: 1, scope: !7, file: !1, line: 2, type: !10)
!12 = !DIExpression()
!13 = !DILocation(line: 2, column: 14, scope: !7)
!14 = !DILocation(line: 3, column: 9, scope: !7)
!15 = !DILocation(line: 3, column: 3, scope: !7)
!16 = !DILocalVariable(name: "a", scope: !7, file: !1, line: 3, type: !17)
!17 = !DICompositeType(tag: DW_TAG_array_type, baseType: !10, elements: !18)
!18 = !{!19}
!19 = !DISubrange(count: -1)
!20 = !DIExpression(DW_OP_deref)
!21 = !DILocation(line: 3, column: 7, scope: !7)
!22 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 3, type: !10)
!23 = !DILocation(line: 3, column: 13, scope: !7)
!24 = !DILocation(line: 4, column: 10, scope: !25)
!25 = distinct !DILexicalBlock(scope: !7, file: !1, line: 4, column: 3)
!26 = !DILocation(line: 4, column: 8, scope: !25)
!27 = !DILocation(line: 4, column: 15, scope: !28)
!28 = !DILexicalBlockFile(scope: !29, file: !1, discriminator: 1)
!29 = distinct !DILexicalBlock(scope: !25, file: !1, line: 4, column: 3)
!30 = !DILocation(line: 4, column: 19, scope: !28)
!31 = !DILocation(line: 4, column: 17, scope: !28)
!32 = !DILocation(line: 4, column: 3, scope: !33)
!33 = !DILexicalBlockFile(scope: !25, file: !1, discriminator: 1)
!34 = !DILocation(line: 5, column: 12, scope: !35)
!35 = distinct !DILexicalBlock(scope: !29, file: !1, line: 4, column: 27)
!36 = !DILocation(line: 5, column: 16, scope: !35)
!37 = !DILocation(line: 5, column: 14, scope: !35)
!38 = !DILocation(line: 5, column: 7, scope: !35)
!39 = !DILocation(line: 5, column: 5, scope: !35)
!40 = !DILocation(line: 5, column: 10, scope: !35)
!41 = !DILocation(line: 6, column: 3, scope: !35)
!42 = !DILocation(line: 4, column: 23, scope: !43)
!43 = !DILexicalBlockFile(scope: !29, file: !1, discriminator: 2)
!44 = !DILocation(line: 4, column: 3, scope: !43)
!45 = distinct !{!45, !46, !47}
!46 = !DILocation(line: 4, column: 3, scope: !25)
!47 = !DILocation(line: 6, column: 3, scope: !25)
!48 = !DILocation(line: 7, column: 10, scope: !7)
!49 = !DILocation(line: 7, column: 14, scope: !7)
!50 = !DILocation(line: 7, column: 12, scope: !7)
!51 = !DILocation(line: 7, column: 5, scope: !7)
!52 = !DILocation(line: 7, column: 3, scope: !7)
!53 = !DILocation(line: 7, column: 8, scope: !7)
!54 = !DILocation(line: 8, column: 1, scope: !7)
!55 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 10, type: !56, isLocal: false, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!56 = !DISubroutineType(types: !57)
!57 = !{!10, !10, !58}
!58 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !59, size: 64)
!59 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !60, size: 64)
!60 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!61 = !DILocalVariable(name: "argc", arg: 1, scope: !55, file: !1, line: 10, type: !10)
!62 = !DILocation(line: 10, column: 14, scope: !55)
!63 = !DILocalVariable(name: "argv", arg: 2, scope: !55, file: !1, line: 10, type: !58)
!64 = !DILocation(line: 10, column: 27, scope: !55)
!65 = !DILocalVariable(name: "v", scope: !55, file: !1, line: 11, type: !10)
!66 = !DILocation(line: 11, column: 7, scope: !55)
!67 = !DILocation(line: 12, column: 3, scope: !55)
!68 = !DILocation(line: 13, column: 7, scope: !55)
!69 = !DILocation(line: 13, column: 3, scope: !55)
!70 = !DILocation(line: 14, column: 3, scope: !55)
