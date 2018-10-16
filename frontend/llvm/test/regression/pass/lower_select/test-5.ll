target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

define internal fastcc i32 @foo(i32 %arg_1, i32 %arg_2) unnamed_addr {
bb_1:
  br label %bb_2

bb_2:                                             ; preds = %bb_3, %bb_1
; CHECK-LABEL: bb_2:                                             ; preds = %bb_3.AfterSelect, %bb_1
; CHECK:   %.01 = phi i32 [ %arg_2, %bb_1 ], [ %..01.phi, %bb_3.AfterSelect ]
; CHECK:   %.0 = phi i32 [ %arg_1, %bb_1 ], [ %_4, %bb_3.AfterSelect ]
; CHECK:   %_1 = icmp slt i32 %.0, 100
; CHECK:   br i1 %_1, label %bb_3, label %bb_4
  %.01 = phi i32 [ %arg_2, %bb_1 ], [ %..01, %bb_3 ]
  %.0 = phi i32 [ %arg_1, %bb_1 ], [ %_4, %bb_3 ]
  %_1 = icmp slt i32 %.0, 100
  br i1 %_1, label %bb_3, label %bb_4

bb_3:                                             ; preds = %bb_2
; CHECK-LABEL: bb_3:
; CHECK:  %_2 = call i32 (...) @__ikos_unknown()
; CHECK:  %_3 = icmp ne i32 %_2, 0
; CHECK:  br i1 %_3, label %bb_3.TrueSelect, label %bb_3.FalseSelect
  %_2 = call i32 (...) @__ikos_unknown() #3
  %_3 = icmp ne i32 %_2, 0
  %..01 = select i1 %_3, i32 1, i32 %.01
  %_4 = add nsw i32 %.0, 4
  br label %bb_2

; CHECK: bb_3.TrueSelect:                                  ; preds = %bb_3
; CHECK:   br label %bb_3.AfterSelect

; CHECK: bb_3.FalseSelect:                                 ; preds = %bb_3
; CHECK:   br label %bb_3.AfterSelect

; CHECK: bb_3.AfterSelect:                                 ; preds = %bb_3.FalseSelect, %bb_3.TrueSelect
; CHECK:   %..01.phi = phi i32 [ 1, %bb_3.TrueSelect ], [ %.01, %bb_3.FalseSelect ]
; CHECK:   %_4 = add nsw i32 %.0, 4
; CHECK:   br label %bb_2

bb_4:                                             ; preds = %bb_2
  %.01.lcssa = phi i32 [ %.01, %bb_2 ]
  %.0.lcssa = phi i32 [ %.0, %bb_2 ]
  %_5 = icmp ult i32 %.01.lcssa, 2
  %_6 = zext i1 %_5 to i32
  call void @__ikos_assert(i32 %_6) #3
  %_7 = icmp slt i32 %.0.lcssa, 104
  %_8 = zext i1 %_7 to i32
  call void @__ikos_assert(i32 %_8) #3
  %_9 = add nsw i32 %.0.lcssa, %.01.lcssa
  ret i32 %_9
}

declare i32 @__ikos_unknown(...) local_unnamed_addr

declare void @__ikos_assert(i32) local_unnamed_addr

define i32 @main() local_unnamed_addr {
bb_1:
  %_1 = call fastcc i32 @foo(i32 0, i32 0)
  %_2 = icmp slt i32 %_1, 105
  %_3 = zext i1 %_2 to i32
  call void @__ikos_assert(i32 %_3) #3
  ret i32 0
}
