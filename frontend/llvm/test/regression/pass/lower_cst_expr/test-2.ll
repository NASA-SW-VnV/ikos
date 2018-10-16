target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main(i32 %arg_1, i8** %arg_2) {
bb_1:
  %_1 = alloca i32, align 4
  %_2 = alloca i32, align 4
  %_3 = alloca i8**, align 8
  %_4 = alloca i32, align 4
  %_5 = alloca i32, align 4
  %_6 = alloca [10 x [10 x i32]], align 16
  store i32 0, i32* %_1, align 4
  store i32 %arg_1, i32* %_2, align 4
  store i8** %arg_2, i8*** %_3, align 8
  store i32 0, i32* %_4, align 4
  br label %bb_2

bb_2:                                             ; preds = %bb_8, %bb_1
  %_7 = load i32, i32* %_4, align 4
  %_8 = icmp slt i32 %_7, 10
  br i1 %_8, label %bb_3, label %bb_9

bb_3:                                             ; preds = %bb_2
  store i32 0, i32* %_5, align 4
  br label %bb_4

bb_4:                                             ; preds = %bb_6, %bb_3
  %_9 = load i32, i32* %_5, align 4
  %_10 = icmp slt i32 %_9, 10
  br i1 %_10, label %bb_5, label %bb_7

bb_5:                                             ; preds = %bb_4
  %_11 = load i32, i32* %_2, align 4
  %_12 = load i32, i32* %_4, align 4
  %_13 = sext i32 %_12 to i64
  %_14 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* %_6, i64 0, i64 %_13
  %_15 = load i32, i32* %_5, align 4
  %_16 = sext i32 %_15 to i64
  %_17 = getelementptr inbounds [10 x i32], [10 x i32]* %_14, i64 0, i64 %_16
  store i32 %_11, i32* %_17, align 4
  br label %bb_6

bb_6:                                             ; preds = %bb_5
  %_18 = load i32, i32* %_5, align 4
  %_19 = add nsw i32 %_18, 1
  store i32 %_19, i32* %_5, align 4
  br label %bb_4

bb_7:                                             ; preds = %bb_4
  br label %bb_8

bb_8:                                             ; preds = %bb_7
  %_20 = load i32, i32* %_4, align 4
  %_21 = add nsw i32 %_20, 1
  store i32 %_21, i32* %_4, align 4
  br label %bb_2

bb_9:                                             ; preds = %bb_2
  store i32 0, i32* %_4, align 4
  br label %bb_10

bb_10:                                            ; preds = %bb_12, %bb_9
  %_22 = load i32, i32* %_4, align 4
  %_23 = icmp slt i32 %_22, 10
  br i1 %_23, label %bb_11, label %bb_13

bb_11:                                            ; preds = %bb_10
; CHECK-LABEL: bb_11:
; CHECK:   %_24 = load i32, i32* %_4, align 4
; CHECK:   %_25 = sext i32 %_24 to i64
; CHECK:   %_26 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* %_6, i64 0, i64 %_25
; CHECK:   %_27 = load i32, i32* %_4, align 4
; CHECK:   %_28 = sext i32 %_27 to i64
; CHECK:   %_29 = getelementptr inbounds [10 x i32], [10 x i32]* %_26, i64 0, i64 %_28
; CHECK:   %_30 = load i32, i32* %_29, align 4
; CHECK:   %0 = getelementptr inbounds [4 x i8], [4 x i8]* @.str, i32 0, i32 0
; CHECK:   %_31 = call i32 (i8*, ...) @printf(i8* %0, i32 %_30)
; CHECK:   br label %bb_12
  %_24 = load i32, i32* %_4, align 4
  %_25 = sext i32 %_24 to i64
  %_26 = getelementptr inbounds [10 x [10 x i32]], [10 x [10 x i32]]* %_6, i64 0, i64 %_25
  %_27 = load i32, i32* %_4, align 4
  %_28 = sext i32 %_27 to i64
  %_29 = getelementptr inbounds [10 x i32], [10 x i32]* %_26, i64 0, i64 %_28
  %_30 = load i32, i32* %_29, align 4
  %_31 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %_30)
  br label %bb_12

bb_12:                                            ; preds = %bb_11
  %_32 = load i32, i32* %_4, align 4
  %_33 = add nsw i32 %_32, 1
  store i32 %_33, i32* %_4, align 4
  br label %bb_10

bb_13:                                            ; preds = %bb_10
  ret i32 0
}

declare i32 @printf(i8*, ...)
