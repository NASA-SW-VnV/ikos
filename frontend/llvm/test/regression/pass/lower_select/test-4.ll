target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

define i32 @main() local_unnamed_addr {
bb_1:
  %_1 = call i8* @malloc(i64 8) #4
  %_2 = icmp eq i8* %_1, null
  br i1 %_2, label %bb_2, label %bb_3

bb_2:                                             ; preds = %bb_1
  call void @exit(i32 0) #5
  unreachable

bb_3:                                             ; preds = %bb_1
; CHECK-LABEL: bb_3:
; CHECK:  %_3 = bitcast i8* %_1 to i32*
; CHECK:  store i32 9, i32* %_3, align 4
; CHECK:  %_4 = getelementptr inbounds i8, i8* %_1, i64 4
; CHECK:  %_5 = bitcast i8* %_4 to i32*
; CHECK:  store i32 20, i32* %_5, align 4
; CHECK:  %_6 = call i32 (...) @__ikos_unknown()
; CHECK:  %_7 = icmp ne i32 %_6, 0
; CHECK:  br i1 %_7, label %bb_3.TrueSelect, label %bb_3.FalseSelect
  %_3 = bitcast i8* %_1 to i32*
  store i32 9, i32* %_3, align 4
  %_4 = getelementptr inbounds i8, i8* %_1, i64 4
  %_5 = bitcast i8* %_4 to i32*
  store i32 20, i32* %_5, align 4
  %_6 = call i32 (...) @__ikos_unknown() #4
  %_7 = icmp ne i32 %_6, 0
  %.sink = select i1 %_7, i32* %_3, i32* %_5
  %_8 = load i32, i32* %.sink, align 4
  %_9 = load i32, i32* %_5, align 4
  %.off = add i32 %_8, -9
  %_10 = icmp ult i32 %.off, 12
  %_11 = zext i1 %_10 to i32
  call void @__ikos_assert(i32 %_11) #4
  %_12 = icmp eq i32 %_9, 20
  %_13 = zext i1 %_12 to i32
  call void @__ikos_assert(i32 %_13) #4
  %_14 = add nsw i32 %_8, %_9
  ret i32 %_14

; CHECK: bb_3.TrueSelect:                                  ; preds = %bb_3
; CHECK:   br label %bb_3.AfterSelect

; CHECK: bb_3.FalseSelect:                                 ; preds = %bb_3
; CHECK:   br label %bb_3.AfterSelect

; CHECK: bb_3.AfterSelect:                                 ; preds = %bb_3.FalseSelect, %bb_3.TrueSelect
; CHECK:   %.sink.phi = phi i32* [ %_3, %bb_3.TrueSelect ], [ %_5, %bb_3.FalseSelect ]
; CHECK:   %_8 = load i32, i32* %.sink.phi, align 4
; CHECK:   %_9 = load i32, i32* %_5, align 4
; CHECK:   %.off = add i32 %_8, -9
; CHECK:   %_10 = icmp ult i32 %.off, 12
; CHECK:   %_11 = zext i1 %_10 to i32
; CHECK:   call void @__ikos_assert(i32 %_11)
; CHECK:   %_12 = icmp eq i32 %_9, 20
; CHECK:   %_13 = zext i1 %_12 to i32
; CHECK:   call void @__ikos_assert(i32 %_13)
; CHECK:   %_14 = add nsw i32 %_8, %_9
; CHECK:   ret i32 %_14
}

declare i8* @malloc(i64) local_unnamed_addr

; Function Attrs: noreturn
declare void @exit(i32) local_unnamed_addr #0

declare i32 @__ikos_unknown(...) local_unnamed_addr

declare void @__ikos_assert(i32) local_unnamed_addr

attributes #0 = { noreturn "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
