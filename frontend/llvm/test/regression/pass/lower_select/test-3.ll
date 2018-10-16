target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

define internal fastcc i32 @foo(i32* %arg_1, i32* %arg_2, i32* %arg_3) unnamed_addr {
bb_1:
; CHECK-LABEL: bb_1:
; CHECK:  %_1 = getelementptr inbounds i32, i32* %arg_1, i64 1
; CHECK:  %_2 = getelementptr inbounds i32, i32* %arg_2, i64 2
; CHECK:  %_3 = icmp eq i32* %_1, %_2
; CHECK:  %_4 = getelementptr inbounds i32, i32* %arg_2, i64 -8
; CHECK:  %_5 = getelementptr inbounds i32, i32* %arg_1, i64 43
; CHECK:  br i1 %_3, label %bb_1.TrueSelect, label %bb_1.FalseSelect
  %_1 = getelementptr inbounds i32, i32* %arg_1, i64 1
  %_2 = getelementptr inbounds i32, i32* %arg_2, i64 2
  %_3 = icmp eq i32* %_1, %_2
  %_4 = getelementptr inbounds i32, i32* %arg_2, i64 -8
  %_5 = getelementptr inbounds i32, i32* %arg_1, i64 43
  %.01 = select i1 %_3, i32* %_5, i32* %_1
  %.0 = select i1 %_3, i32* %_4, i32* %_2
  %_6 = load i32, i32* %.01, align 4
  %_7 = icmp eq i32 %_6, 3
  %_8 = zext i1 %_7 to i32
  call void @__ikos_assert(i32 %_8) #3
  %_9 = load i32, i32* %.0, align 4
  %_10 = icmp eq i32 %_9, 6
  %_11 = zext i1 %_10 to i32
  call void @__ikos_assert(i32 %_11) #3
  %_12 = load i32, i32* %.01, align 4
  %_13 = load i32, i32* %.0, align 4
  %_14 = add nsw i32 %_12, %_13
  %_15 = sext i32 %_14 to i64
  %_16 = getelementptr inbounds i32, i32* %arg_3, i64 %_15
  %_17 = load i32, i32* %_16, align 4
  store i32 555, i32* %_16, align 4
  ret i32 %_17

; CHECK: bb_1.TrueSelect:                                  ; preds = %bb_1
; CHECK:   br label %bb_1.AfterSelect

; CHECK: bb_1.FalseSelect:                                 ; preds = %bb_1
; CHECK:   br label %bb_1.AfterSelect

; CHECK: bb_1.AfterSelect:                                 ; preds = %bb_1.FalseSelect, %bb_1.TrueSelect
; CHECK:   %.01.phi = phi i32* [ %_5, %bb_1.TrueSelect ], [ %_1, %bb_1.FalseSelect ]
; CHECK:   br i1 %_3, label %bb_1.AfterSelect.TrueSelect, label %bb_1.AfterSelect.FalseSelect

; CHECK: bb_1.AfterSelect.TrueSelect:                      ; preds = %bb_1.AfterSelect
; CHECK:   br label %bb_1.AfterSelect.AfterSelect

; CHECK: bb_1.AfterSelect.FalseSelect:                     ; preds = %bb_1.AfterSelect
; CHECK:   br label %bb_1.AfterSelect.AfterSelect

; CHECK: bb_1.AfterSelect.AfterSelect:                     ; preds = %bb_1.AfterSelect.FalseSelect, %bb_1.AfterSelect.TrueSelect
; CHECK:   %.0.phi = phi i32* [ %_4, %bb_1.AfterSelect.TrueSelect ], [ %_2, %bb_1.AfterSelect.FalseSelect ]
; CHECK:   %_6 = load i32, i32* %.01.phi, align 4
; CHECK:   %_7 = icmp eq i32 %_6, 3
; CHECK:   %_8 = zext i1 %_7 to i32
; CHECK:   call void @__ikos_assert(i32 %_8)
; CHECK:   %_9 = load i32, i32* %.0.phi, align 4
; CHECK:   %_10 = icmp eq i32 %_9, 6
; CHECK:   %_11 = zext i1 %_10 to i32
; CHECK:   call void @__ikos_assert(i32 %_11)
; CHECK:   %_12 = load i32, i32* %.01.phi, align 4
; CHECK:   %_13 = load i32, i32* %.0.phi, align 4
; CHECK:   %_14 = add nsw i32 %_12, %_13
; CHECK:   %_15 = sext i32 %_14 to i64
; CHECK:   %_16 = getelementptr inbounds i32, i32* %arg_3, i64 %_15
; CHECK:   %_17 = load i32, i32* %_16, align 4
; CHECK:   store i32 555, i32* %_16, align 4
; CHECK:   ret i32 %_17
}

declare void @__ikos_assert(i32) local_unnamed_addr

define i32 @main(i32 %arg_1, i8** %arg_2) local_unnamed_addr {
bb_1:
  %_1 = alloca [2 x i32], align 4
  %_2 = alloca [3 x i32], align 4
  %_3 = alloca [10 x i32], align 16
  %_4 = getelementptr inbounds [10 x i32], [10 x i32]* %_3, i64 0, i64 9
  store i32 666, i32* %_4, align 4
  %_5 = getelementptr inbounds [2 x i32], [2 x i32]* %_1, i64 0, i64 0
  store i32 1, i32* %_5, align 4
  %_6 = getelementptr inbounds [2 x i32], [2 x i32]* %_1, i64 0, i64 1
  store i32 3, i32* %_6, align 4
  %_7 = getelementptr inbounds [3 x i32], [3 x i32]* %_2, i64 0, i64 0
  store i32 4, i32* %_7, align 4
  %_8 = getelementptr inbounds [3 x i32], [3 x i32]* %_2, i64 0, i64 1
  store i32 5, i32* %_8, align 4
  %_9 = getelementptr inbounds [3 x i32], [3 x i32]* %_2, i64 0, i64 2
  store i32 6, i32* %_9, align 4
  %_10 = getelementptr inbounds [2 x i32], [2 x i32]* %_1, i64 0, i64 0
  %_11 = getelementptr inbounds [3 x i32], [3 x i32]* %_2, i64 0, i64 0
  %_12 = getelementptr inbounds [10 x i32], [10 x i32]* %_3, i64 0, i64 0
  %_13 = call fastcc i32 @foo(i32* %_10, i32* %_11, i32* %_12)
  %_14 = icmp eq i32 %_13, 666
  %_15 = zext i1 %_14 to i32
  call void @__ikos_assert(i32 %_15) #3
  %_16 = load i32, i32* %_4, align 4
  %_17 = icmp eq i32 %_16, 555
  %_18 = zext i1 %_17 to i32
  call void @__ikos_assert(i32 %_18) #3
  ret i32 %_13
}
