target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

define internal fastcc i32 @foo(float* %arg_1, i32 %arg_2, float %arg_3) unnamed_addr {
bb_1:
  %_1 = add nsw i32 %arg_2, -1
  %_2 = load float, float* %arg_1, align 4
  %_3 = fcmp ult float %_2, %arg_3
  br i1 %_3, label %bb_2, label %.thread

bb_2:                                             ; preds = %bb_1
; CHECK-LABEL: bb_2:
; CHECK:  %_4 = sext i32 %_1 to i64
; CHECK:  %_5 = getelementptr inbounds float, float* %arg_1, i64 %_4
; CHECK:  %_6 = load float, float* %_5, align 4
; CHECK:  %_7 = fcmp ole float %_6, %arg_3
; CHECK:  %_8 = add nsw i32 %arg_2, -2
; CHECK:  br i1 %_7, label %bb_2.TrueSelect, label %bb_2.FalseSelect
  %_4 = sext i32 %_1 to i64
  %_5 = getelementptr inbounds float, float* %arg_1, i64 %_4
  %_6 = load float, float* %_5, align 4
  %_7 = fcmp ole float %_6, %arg_3
  %_8 = add nsw i32 %arg_2, -2
  %.01 = select i1 %_7, i32 %_8, i32 0
  %phitmp = xor i1 %_7, true
  br i1 %phitmp, label %bb_3, label %.thread

; CHECK: bb_2.TrueSelect:                                  ; preds = %bb_2
; CHECK:   br label %bb_2.AfterSelect

; CHECK: bb_2.FalseSelect:                                 ; preds = %bb_2
; CHECK:   br label %bb_2.AfterSelect

; CHECK: bb_2.AfterSelect:                                 ; preds = %bb_2.FalseSelect, %bb_2.TrueSelect
; CHECK:   %.01.phi = phi i32 [ %_8, %bb_2.TrueSelect ], [ 0, %bb_2.FalseSelect ]
; CHECK:   %phitmp = xor i1 %_7, true
; CHECK:   br i1 %phitmp, label %bb_3, label %.thread

bb_3:                                             ; preds = %bb_2
; CHECK-LABEL: bb_3:                                             ; preds = %bb_2.AfterSelect
; CHECK:  %_9 = fcmp olt float %arg_3, 0.000000e+00
; CHECK:  br i1 %_9, label %.outer, label %.outer13
  %_9 = fcmp olt float %arg_3, 0.000000e+00
  br i1 %_9, label %.outer, label %.outer13

.outer:                                           ; preds = %bb_6, %bb_3
  %.07.ph = phi i32 [ %_17, %bb_6 ], [ 0, %bb_3 ]
  %.03.ph = phi i32 [ %.03.lcssa, %bb_6 ], [ %_1, %bb_3 ]
  br label %bb_4

bb_4:                                             ; preds = %bb_5, %.outer
  %.03 = phi i32 [ %_16, %bb_5 ], [ %.03.ph, %.outer ]
  %_10 = add nsw i32 %.07.ph, %.03
  %_11 = sdiv i32 %_10, 2
  %_12 = sext i32 %_11 to i64
  %_13 = getelementptr inbounds float, float* %arg_1, i64 %_12
  %_14 = load float, float* %_13, align 4
  %_15 = fcmp ogt float %_14, %arg_3
  br i1 %_15, label %bb_5, label %bb_6

bb_5:                                             ; preds = %bb_4
  %_16 = add nsw i32 %_11, -1
  br label %bb_4

bb_6:                                             ; preds = %bb_4
  %.03.lcssa = phi i32 [ %.03, %bb_4 ]
  %.lcssa = phi i32 [ %_11, %bb_4 ]
  %_17 = add nsw i32 %.lcssa, 1
  %_18 = sext i32 %_17 to i64
  %_19 = getelementptr inbounds float, float* %arg_1, i64 %_18
  %_20 = load float, float* %_19, align 4
  %_21 = fcmp ugt float %_20, %arg_3
  br i1 %_21, label %.thread, label %.outer

bb_7:                                             ; preds = %.outer13, %bb_8
  %.25 = phi i32 [ %_28, %bb_8 ], [ %.25.ph, %.outer13 ]
  %_22 = add nsw i32 %.29.ph, %.25
  %_23 = sdiv i32 %_22, 2
  %_24 = sext i32 %_23 to i64
  %_25 = getelementptr inbounds float, float* %arg_1, i64 %_24
  %_26 = load float, float* %_25, align 4
  %_27 = fcmp ult float %_26, %arg_3
  br i1 %_27, label %bb_9, label %bb_8

bb_8:                                             ; preds = %bb_7
  %_28 = add nsw i32 %_23, -1
  br label %bb_7

bb_9:                                             ; preds = %bb_7
  %.25.lcssa = phi i32 [ %.25, %bb_7 ]
  %.lcssa14 = phi i32 [ %_23, %bb_7 ]
  %_29 = add nsw i32 %.lcssa14, 1
  %_30 = sext i32 %_29 to i64
  %_31 = getelementptr inbounds float, float* %arg_1, i64 %_30
  %_32 = load float, float* %_31, align 4
  %_33 = fcmp olt float %_32, %arg_3
  br i1 %_33, label %.outer13, label %.thread

.outer13:                                         ; preds = %bb_9, %bb_3
  %.29.ph = phi i32 [ %_29, %bb_9 ], [ 0, %bb_3 ]
  %.25.ph = phi i32 [ %.25.lcssa, %bb_9 ], [ %_1, %bb_3 ]
  br label %bb_7

.thread:                                          ; preds = %bb_9, %bb_6, %bb_2, %bb_1
; CHECK-LABEL: .thread:                                          ; preds = %bb_9, %bb_6, %bb_2.AfterSelect, %bb_1
; CHECK:   %.3 = phi i32 [ %.01.phi, %bb_2.AfterSelect ], [ 0, %bb_1 ], [ %.lcssa, %bb_6 ], [ %.lcssa14, %bb_9 ]
; CHECK:   ret i32 %.3
  %.3 = phi i32 [ %.01, %bb_2 ], [ 0, %bb_1 ], [ %.lcssa, %bb_6 ], [ %.lcssa14, %bb_9 ]
  ret i32 %.3
}

define i32 @main(i32 %arg_1, i8** %arg_2) local_unnamed_addr {
bb_1:
  %_1 = alloca [100 x float], align 16
  %.sub = getelementptr inbounds [100 x float], [100 x float]* %_1, i64 0, i64 0
  %_2 = call fastcc i32 @foo(float* %.sub, i32 100, float 3.400000e+01)
  ret i32 %_2
}
