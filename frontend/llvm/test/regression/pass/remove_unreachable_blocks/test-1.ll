target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

define i32 @main() {
bb_1:
  %_1 = alloca i32, align 4
  %_2 = alloca i32, align 4
  store i32 0, i32* %_1, align 4
  store i32 0, i32* %_2, align 4
  br label %bb_2

bb_2:                                             ; preds = %bb_9, %bb_3, %bb_1
  %_3 = call i32 (...) @__ikos_unknown()
  %_4 = icmp ne i32 %_3, 0
  br i1 %_4, label %bb_3, label %bb_4

bb_3:                                             ; preds = %bb_2
  br label %bb_2

bb_4:                                             ; preds = %bb_2
  %_5 = load i32, i32* %_2, align 4
  %_6 = icmp slt i32 %_5, 60
  br i1 %_6, label %bb_5, label %bb_6

bb_5:                                             ; preds = %bb_4
  %_7 = load i32, i32* %_2, align 4
  %_8 = add nsw i32 %_7, 1
  store i32 %_8, i32* %_2, align 4
  br label %bb_7

bb_6:                                             ; preds = %bb_4
  store i32 0, i32* %_2, align 4
  br label %bb_7

bb_7:                                             ; preds = %bb_6, %bb_5
  %_9 = load i32, i32* %_2, align 4
  %_10 = icmp sge i32 %_9, 0
  br i1 %_10, label %bb_8, label %bb_9

bb_8:                                             ; preds = %bb_7
  %_11 = load i32, i32* %_2, align 4
  %_12 = icmp sle i32 %_11, 60
  br label %bb_9

bb_9:                                             ; preds = %bb_8, %bb_7
  %_13 = phi i1 [ false, %bb_7 ], [ %_12, %bb_8 ]
  %_14 = zext i1 %_13 to i32
  call void @__ikos_assert(i32 %_14)
  br label %bb_2

bb_10:                                            ; No predecessors!
; CHECK-NOT: bb_10:
; CHECK-NOT:  %_15 = load i32, i32* %_1, align 4
; CHECK-NOT:  ret i32 %_15
  %_15 = load i32, i32* %_1, align 4
  ret i32 %_15
}

declare i32 @__ikos_unknown(...)

declare void @__ikos_assert(i32)
