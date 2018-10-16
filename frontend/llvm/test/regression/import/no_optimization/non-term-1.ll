; ModuleID = 'non-term-1.c.pp.bc'
source_filename = "non-term-1.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

; Function Attrs: noreturn
declare void @exit(i32) #2
; CHECK: declare void @ar.libc.exit(si32)

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main(i32, i8**) #0 !dbg !7 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i8**, align 8
  %6 = alloca i32, align 4
  store i32 0, i32* %3, align 4
  store i32 %0, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !14, metadata !15), !dbg !16
  store i8** %1, i8*** %5, align 8
  call void @llvm.dbg.declare(metadata i8*** %5, metadata !17, metadata !15), !dbg !18
  call void @llvm.dbg.declare(metadata i32* %6, metadata !19, metadata !15), !dbg !20
  br label %7, !dbg !21

; <label>:7:                                      ; preds = %2, %11
  %8 = load i32, i32* %6, align 4, !dbg !22
  %9 = icmp eq i32 %8, 0, !dbg !25
  br i1 %9, label %10, label %11, !dbg !26

; <label>:10:                                     ; preds = %7
  call void @exit(i32 1) #3, !dbg !27
  unreachable, !dbg !27

; <label>:11:                                     ; preds = %7
  %12 = load i32, i32* %6, align 4, !dbg !28
  %13 = add nsw i32 %12, 1, !dbg !28
  store i32 %13, i32* %6, align 4, !dbg !28
  br label %7, !dbg !29, !llvm.loop !31
                                                  ; No predecessors!
  %15 = load i32, i32* %3, align 4, !dbg !33
  ret i32 %15, !dbg !33
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
; CHECK: }
; CHECK: #2 predecessors={#1, #4} successors={#3, #4} {
; CHECK:   si32 %7 = load $6, align 4
; CHECK: }
; CHECK: #3 !unreachable predecessors={#2} {
; CHECK:   %7 sieq 0
; CHECK:   call @ar.libc.exit(1)
; CHECK:   unreachable
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#2} {
; CHECK:   %7 sine 0
; CHECK:   si32 %8 = load $6, align 4
; CHECK:   si32 %9 = %8 sadd.nw 1
; CHECK:   store $6, %9, align 4
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "non-term-1.c", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !8, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10, !10, !11}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!13 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!14 = !DILocalVariable(name: "argc", arg: 1, scope: !7, file: !1, line: 3, type: !10)
!15 = !DIExpression()
!16 = !DILocation(line: 3, column: 14, scope: !7)
!17 = !DILocalVariable(name: "argv", arg: 2, scope: !7, file: !1, line: 3, type: !11)
!18 = !DILocation(line: 3, column: 27, scope: !7)
!19 = !DILocalVariable(name: "i", scope: !7, file: !1, line: 4, type: !10)
!20 = !DILocation(line: 4, column: 7, scope: !7)
!21 = !DILocation(line: 5, column: 3, scope: !7)
!22 = !DILocation(line: 6, column: 9, scope: !23)
!23 = distinct !DILexicalBlock(scope: !24, file: !1, line: 6, column: 9)
!24 = distinct !DILexicalBlock(scope: !7, file: !1, line: 5, column: 13)
!25 = !DILocation(line: 6, column: 11, scope: !23)
!26 = !DILocation(line: 6, column: 9, scope: !24)
!27 = !DILocation(line: 7, column: 7, scope: !23)
!28 = !DILocation(line: 8, column: 6, scope: !24)
!29 = !DILocation(line: 5, column: 3, scope: !30)
!30 = !DILexicalBlockFile(scope: !7, file: !1, discriminator: 1)
!31 = distinct !{!31, !21, !32}
!32 = !DILocation(line: 9, column: 3, scope: !7)
!33 = !DILocation(line: 10, column: 1, scope: !7)
