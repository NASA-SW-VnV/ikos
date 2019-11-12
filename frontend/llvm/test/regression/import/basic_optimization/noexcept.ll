; ModuleID = 'noexcept.pp.bc'
source_filename = "noexcept.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

declare i32 @_Z1fv() #1
; CHECK: declare si32 @_Z1fv()

; Function Attrs: noinline nounwind ssp uwtable
define i32 @_Z1gv() #0 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !dbg !8 {
  %1 = invoke i32 @_Z1fv()
          to label %2 unwind label %3, !dbg !12

2:                                                ; preds = %0
  ret i32 %1, !dbg !13

3:                                                ; preds = %0
  %4 = landingpad { i8*, i32 }
          catch i8* null, !dbg !12
  %5 = extractvalue { i8*, i32 } %4, 0, !dbg !12
  call void @__clang_call_terminate(i8* %5) #3, !dbg !12
  unreachable, !dbg !12
}
; CHECK: define si32 @_Z1gv() {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32 %1 = invoke @_Z1fv() normal=#2 exc=#3
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#unified-exit} {
; CHECK:   return %1
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#unified-exit} {
; CHECK:   {0: si8*, 8: si32} %2 = landingpad
; CHECK:   si8* %3 = extractelement %2, 0
; CHECK:   call @__clang_call_terminate(%3)
; CHECK:   unreachable
; CHECK: }
; CHECK: #unified-exit !exit predecessors={#2, #3} {
; CHECK: }
; CHECK: }

declare void @_ZSt9terminatev()
; CHECK: declare void @_ZSt9terminatev()

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(i8*) #2 {
  %2 = call i8* @__cxa_begin_catch(i8* %0) #4
  call void @_ZSt9terminatev() #3
  unreachable
}
; CHECK: define void @__clang_call_terminate(si8* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %2 = call @ar.libcpp.begincatch(%1)
; CHECK:   call @_ZSt9terminatev()
; CHECK:   unreachable
; CHECK: }
; CHECK: }

declare i32 @__gxx_personality_v0(...)
; CHECK: declare si32 @__gxx_personality_v0(...)

declare i8* @__cxa_begin_catch(i8*)
; CHECK: declare si8* @ar.libcpp.begincatch(si8*)

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline noreturn nounwind }
attributes #3 = { noreturn nounwind }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "noexcept.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "g", linkageName: "_Z1gv", scope: !1, file: !1, line: 3, type: !9, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocation(line: 4, column: 10, scope: !8)
!13 = !DILocation(line: 4, column: 3, scope: !8)
