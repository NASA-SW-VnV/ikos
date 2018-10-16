; ModuleID = 'local-array-2.c.pp.bc'
source_filename = "local-array-2.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

@.str = private unnamed_addr constant [34 x i8] c"This is string.h library function\00", align 1
; CHECK: define [34 x si8]* @.str, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @.str, [84, 104, 105, 115, 32, 105, 115, 32, 115, 116, 114, 105, 110, 103, 46, 104, 32, 108, 105, 98, 114, 97, 114, 121, 32, 102, 117, 110, 99, 116, 105, 111, 110, 0], align 1
; CHECK: }
; CHECK: }

declare i32 @puts(i8*) #2
; CHECK: declare si32 @ar.libc.puts(si8*)

declare i8* @strcpy(i8*, i8*) #2
; CHECK: declare si8* @ar.libc.strcpy(si8*, si8*)

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i32, i1) #3
; CHECK: declare void @ar.memcpy(si8*, si8*, ui64, ui32, ui1)

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i32, i1) #3
; CHECK: declare void @ar.memset(si8*, si8, ui64, ui32, ui1)

; Function Attrs: noinline nounwind ssp uwtable
define i8* @foo(i8*, i32) #0 !dbg !7 {
  %3 = alloca i8*, align 8
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  store i8* %0, i8** %3, align 8
  call void @llvm.dbg.declare(metadata i8** %3, metadata !13, metadata !14), !dbg !15
  store i32 %1, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !16, metadata !14), !dbg !17
  call void @llvm.dbg.declare(metadata i32* %5, metadata !18, metadata !14), !dbg !19
  store i32 0, i32* %5, align 4, !dbg !20
  br label %6, !dbg !22

; <label>:6:                                      ; preds = %15, %2
  %7 = load i32, i32* %5, align 4, !dbg !23
  %8 = load i32, i32* %4, align 4, !dbg !26
  %9 = icmp slt i32 %7, %8, !dbg !27
  br i1 %9, label %10, label %18, !dbg !28

; <label>:10:                                     ; preds = %6
  %11 = load i8*, i8** %3, align 8, !dbg !30
  %12 = load i32, i32* %5, align 4, !dbg !31
  %13 = sext i32 %12 to i64, !dbg !30
  %14 = getelementptr inbounds i8, i8* %11, i64 %13, !dbg !30
  store i8 65, i8* %14, align 1, !dbg !32
  br label %15, !dbg !30

; <label>:15:                                     ; preds = %10
  %16 = load i32, i32* %5, align 4, !dbg !33
  %17 = add nsw i32 %16, 1, !dbg !33
  store i32 %17, i32* %5, align 4, !dbg !33
  br label %6, !dbg !35, !llvm.loop !36

; <label>:18:                                     ; preds = %6
  %19 = load i8*, i8** %3, align 8, !dbg !39
  ret i8* %19, !dbg !40
}
; CHECK: define si8* @foo(si8* %1, si32 %2) {
; CHECK: #1 !entry successors={#2} {
; CHECK:   si8** $3 = allocate si8*, 1, align 8
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   si32* $5 = allocate si32, 1, align 4
; CHECK:   store $3, %1, align 8
; CHECK:   store $4, %2, align 4
; CHECK:   store $5, 0, align 4
; CHECK: }
; CHECK: #2 predecessors={#1, #3} successors={#3, #4} {
; CHECK:   si32 %6 = load $5, align 4
; CHECK:   si32 %7 = load $4, align 4
; CHECK: }
; CHECK: #3 predecessors={#2} successors={#2} {
; CHECK:   %6 silt %7
; CHECK:   si8* %8 = load $3, align 8
; CHECK:   si32 %9 = load $5, align 4
; CHECK:   si64 %10 = sext %9
; CHECK:   si8* %11 = ptrshift %8, 1 * %10
; CHECK:   store %11, 65, align 1
; CHECK:   si32 %12 = load $5, align 4
; CHECK:   si32 %13 = %12 sadd.nw 1
; CHECK:   store $5, %13, align 4
; CHECK: }
; CHECK: #4 !exit predecessors={#2} {
; CHECK:   %6 sige %7
; CHECK:   si8* %14 = load $3, align 8
; CHECK:   return %14
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 !dbg !41 {
  %1 = alloca i32, align 4
  %2 = alloca [50 x i8], align 16
  %3 = alloca i8*, align 8
  %4 = alloca [10 x i8], align 1
  %5 = alloca i8*, align 8
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata [50 x i8]* %2, metadata !44, metadata !14), !dbg !48
  %6 = getelementptr inbounds [50 x i8], [50 x i8]* %2, i32 0, i32 0, !dbg !49
  %7 = getelementptr inbounds [34 x i8], [34 x i8]* @.str, i32 0, i32 0, !dbg !50
  %8 = call i8* @strcpy(i8* %6, i8* %7), !dbg !50
  %9 = getelementptr inbounds [50 x i8], [50 x i8]* %2, i32 0, i32 0, !dbg !51
  %10 = call i32 @puts(i8* %9), !dbg !52
  %11 = getelementptr inbounds [50 x i8], [50 x i8]* %2, i32 0, i32 0, !dbg !53
  call void @llvm.memset.p0i8.i64(i8* %11, i8 36, i64 50, i32 16, i1 false), !dbg !53
  call void @llvm.dbg.declare(metadata i8** %3, metadata !54, metadata !14), !dbg !55
  %12 = getelementptr inbounds [50 x i8], [50 x i8]* %2, i32 0, i32 0, !dbg !56
  %13 = call i8* @foo(i8* %12, i32 10), !dbg !57
  store i8* %13, i8** %3, align 8, !dbg !55
  call void @llvm.dbg.declare(metadata [10 x i8]* %4, metadata !58, metadata !14), !dbg !62
  %14 = getelementptr inbounds [10 x i8], [10 x i8]* %4, i32 0, i32 0, !dbg !63
  %15 = load i8*, i8** %3, align 8, !dbg !64
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %14, i8* %15, i64 10, i32 1, i1 false), !dbg !63
  call void @llvm.dbg.declare(metadata i8** %5, metadata !65, metadata !14), !dbg !66
  %16 = getelementptr inbounds [10 x i8], [10 x i8]* %4, i32 0, i32 0, !dbg !67
  %17 = call i8* @foo(i8* %16, i32 10), !dbg !68
  store i8* %17, i8** %5, align 8, !dbg !66
  %18 = load i8*, i8** %5, align 8, !dbg !69
  %19 = call i32 @puts(i8* %18), !dbg !70
  ret i32 0, !dbg !71
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   [50 x si8]* $2 = allocate [50 x si8], 1, align 16
; CHECK:   si8** $3 = allocate si8*, 1, align 8
; CHECK:   [10 x si8]* $4 = allocate [10 x si8], 1, align 1
; CHECK:   si8** $5 = allocate si8*, 1, align 8
; CHECK:   store $1, 0, align 4
; CHECK:   si8* %6 = ptrshift $2, 50 * 0, 1 * 0
; CHECK:   si8* %7 = ptrshift @.str, 34 * 0, 1 * 0
; CHECK:   si8* %8 = call @ar.libc.strcpy(%6, %7)
; CHECK:   si8* %9 = ptrshift $2, 50 * 0, 1 * 0
; CHECK:   si32 %10 = call @ar.libc.puts(%9)
; CHECK:   si8* %11 = ptrshift $2, 50 * 0, 1 * 0
; CHECK:   call @ar.memset(%11, 36, 50, 16, 0)
; CHECK:   si8* %12 = ptrshift $2, 50 * 0, 1 * 0
; CHECK:   si8* %13 = call @foo(%12, 10)
; CHECK:   store $3, %13, align 8
; CHECK:   si8* %14 = ptrshift $4, 10 * 0, 1 * 0
; CHECK:   si8* %15 = load $3, align 8
; CHECK:   call @ar.memcpy(%14, %15, 10, 1, 0)
; CHECK:   si8* %16 = ptrshift $4, 10 * 0, 1 * 0
; CHECK:   si8* %17 = call @foo(%16, 10)
; CHECK:   store $5, %17, align 8
; CHECK:   si8* %18 = load $5, align 8
; CHECK:   si32 %19 = call @ar.libc.puts(%18)
; CHECK:   return 0
; CHECK: }
; CHECK: }

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { argmemonly nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "local-array-2.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 4, type: !8, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !12}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!12 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!13 = !DILocalVariable(name: "a", arg: 1, scope: !7, file: !1, line: 4, type: !10)
!14 = !DIExpression()
!15 = !DILocation(line: 4, column: 17, scope: !7)
!16 = !DILocalVariable(name: "n", arg: 2, scope: !7, file: !1, line: 4, type: !12)
!17 = !DILocation(line: 4, column: 24, scope: !7)
!18 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 5, type: !12)
!19 = !DILocation(line: 5, column: 7, scope: !7)
!20 = !DILocation(line: 6, column: 10, scope: !21)
!21 = distinct !DILexicalBlock(scope: !7, file: !1, line: 6, column: 3)
!22 = !DILocation(line: 6, column: 8, scope: !21)
!23 = !DILocation(line: 6, column: 15, scope: !24)
!24 = !DILexicalBlockFile(scope: !25, file: !1, discriminator: 1)
!25 = distinct !DILexicalBlock(scope: !21, file: !1, line: 6, column: 3)
!26 = !DILocation(line: 6, column: 19, scope: !24)
!27 = !DILocation(line: 6, column: 17, scope: !24)
!28 = !DILocation(line: 6, column: 3, scope: !29)
!29 = !DILexicalBlockFile(scope: !21, file: !1, discriminator: 1)
!30 = !DILocation(line: 7, column: 5, scope: !25)
!31 = !DILocation(line: 7, column: 7, scope: !25)
!32 = !DILocation(line: 7, column: 10, scope: !25)
!33 = !DILocation(line: 6, column: 23, scope: !34)
!34 = !DILexicalBlockFile(scope: !25, file: !1, discriminator: 2)
!35 = !DILocation(line: 6, column: 3, scope: !34)
!36 = distinct !{!36, !37, !38}
!37 = !DILocation(line: 6, column: 3, scope: !21)
!38 = !DILocation(line: 7, column: 12, scope: !21)
!39 = !DILocation(line: 8, column: 10, scope: !7)
!40 = !DILocation(line: 8, column: 3, scope: !7)
!41 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 11, type: !42, isLocal: false, isDefinition: true, scopeLine: 11, isOptimized: false, unit: !0, variables: !2)
!42 = !DISubroutineType(types: !43)
!43 = !{!12}
!44 = !DILocalVariable(name: "str", scope: !41, file: !1, line: 12, type: !45)
!45 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 400, elements: !46)
!46 = !{!47}
!47 = !DISubrange(count: 50)
!48 = !DILocation(line: 12, column: 8, scope: !41)
!49 = !DILocation(line: 14, column: 10, scope: !41)
!50 = !DILocation(line: 14, column: 3, scope: !41)
!51 = !DILocation(line: 15, column: 8, scope: !41)
!52 = !DILocation(line: 15, column: 3, scope: !41)
!53 = !DILocation(line: 17, column: 3, scope: !41)
!54 = !DILocalVariable(name: "A", scope: !41, file: !1, line: 18, type: !10)
!55 = !DILocation(line: 18, column: 9, scope: !41)
!56 = !DILocation(line: 18, column: 17, scope: !41)
!57 = !DILocation(line: 18, column: 13, scope: !41)
!58 = !DILocalVariable(name: "B", scope: !41, file: !1, line: 20, type: !59)
!59 = !DICompositeType(tag: DW_TAG_array_type, baseType: !11, size: 80, elements: !60)
!60 = !{!61}
!61 = !DISubrange(count: 10)
!62 = !DILocation(line: 20, column: 8, scope: !41)
!63 = !DILocation(line: 21, column: 3, scope: !41)
!64 = !DILocation(line: 21, column: 13, scope: !41)
!65 = !DILocalVariable(name: "C", scope: !41, file: !1, line: 22, type: !10)
!66 = !DILocation(line: 22, column: 9, scope: !41)
!67 = !DILocation(line: 22, column: 17, scope: !41)
!68 = !DILocation(line: 22, column: 13, scope: !41)
!69 = !DILocation(line: 23, column: 8, scope: !41)
!70 = !DILocation(line: 23, column: 3, scope: !41)
!71 = !DILocation(line: 24, column: 3, scope: !41)
