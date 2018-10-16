target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

%struct.foo = type { i32, %struct.bar, [10 x [10 x [9 x i32]]] }
%struct.bar = type { i32, float }

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main(i32 %arg_1, i8** %arg_2) {
bb_1:
  %_1 = alloca i32, align 4
  %_2 = alloca i32, align 4
  %_3 = alloca i8**, align 8
  %_4 = alloca i32, align 4
  %_5 = alloca i32, align 4
  %_6 = alloca i32, align 4
  %_7 = alloca %struct.foo, align 4
  store i32 0, i32* %_1, align 4
  store i32 %arg_1, i32* %_2, align 4
  store i8** %arg_2, i8*** %_3, align 8
  store i32 0, i32* %_4, align 4
  br label %bb_2

bb_2:                                             ; preds = %bb_12, %bb_1
  %_8 = load i32, i32* %_4, align 4
  %_9 = icmp slt i32 %_8, 10
  br i1 %_9, label %bb_3, label %bb_13

bb_3:                                             ; preds = %bb_2
  store i32 0, i32* %_5, align 4
  br label %bb_4

bb_4:                                             ; preds = %bb_10, %bb_3
  %_10 = load i32, i32* %_5, align 4
  %_11 = icmp slt i32 %_10, 10
  br i1 %_11, label %bb_5, label %bb_11

bb_5:                                             ; preds = %bb_4
  store i32 0, i32* %_6, align 4
  br label %bb_6

bb_6:                                             ; preds = %bb_8, %bb_5
  %_12 = load i32, i32* %_6, align 4
  %_13 = icmp slt i32 %_12, 9
  br i1 %_13, label %bb_7, label %bb_9

bb_7:                                             ; preds = %bb_6
  %_14 = load i32, i32* %_2, align 4
  %_15 = getelementptr inbounds %struct.foo, %struct.foo* %_7, i32 0, i32 2
  %_16 = load i32, i32* %_4, align 4
  %_17 = sext i32 %_16 to i64
  %_18 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %_15, i64 0, i64 %_17
  %_19 = load i32, i32* %_5, align 4
  %_20 = sext i32 %_19 to i64
  %_21 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %_18, i64 0, i64 %_20
  %_22 = load i32, i32* %_6, align 4
  %_23 = sext i32 %_22 to i64
  %_24 = getelementptr inbounds [9 x i32], [9 x i32]* %_21, i64 0, i64 %_23
  store i32 %_14, i32* %_24, align 4
  %_25 = getelementptr inbounds %struct.foo, %struct.foo* %_7, i32 0, i32 2
  %_26 = load i32, i32* %_4, align 4
  %_27 = sext i32 %_26 to i64
  %_28 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %_25, i64 0, i64 %_27
  %_29 = load i32, i32* %_5, align 4
  %_30 = sext i32 %_29 to i64
  %_31 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %_28, i64 0, i64 %_30
  %_32 = load i32, i32* %_6, align 4
  %_33 = sext i32 %_32 to i64
  %_34 = getelementptr inbounds [9 x i32], [9 x i32]* %_31, i64 0, i64 %_33
  %_35 = load i32, i32* %_34, align 4
  %_36 = getelementptr inbounds %struct.foo, %struct.foo* %_7, i32 0, i32 1
  %_37 = getelementptr inbounds %struct.bar, %struct.bar* %_36, i32 0, i32 0
  store i32 %_35, i32* %_37, align 4
  br label %bb_8

bb_8:                                             ; preds = %bb_7
  %_38 = load i32, i32* %_6, align 4
  %_39 = add nsw i32 %_38, 1
  store i32 %_39, i32* %_6, align 4
  br label %bb_6

bb_9:                                             ; preds = %bb_6
  br label %bb_10

bb_10:                                            ; preds = %bb_9
  %_40 = load i32, i32* %_5, align 4
  %_41 = add nsw i32 %_40, 1
  store i32 %_41, i32* %_5, align 4
  br label %bb_4

bb_11:                                            ; preds = %bb_4
  br label %bb_12

bb_12:                                            ; preds = %bb_11
  %_42 = load i32, i32* %_4, align 4
  %_43 = add nsw i32 %_42, 1
  store i32 %_43, i32* %_4, align 4
  br label %bb_2

bb_13:                                            ; preds = %bb_2
  store i32 0, i32* %_4, align 4
  br label %bb_14

bb_14:                                            ; preds = %bb_16, %bb_13
  %_44 = load i32, i32* %_4, align 4
  %_45 = icmp slt i32 %_44, 10
  br i1 %_45, label %bb_15, label %bb_17

bb_15:                                            ; preds = %bb_14
; CHECK-LABEL: b_15:
; CHECK:  %_46 = getelementptr inbounds %struct.foo, %struct.foo* %_7, i32 0, i32 2
; CHECK:  %_47 = load i32, i32* %_4, align 4
; CHECK:  %_48 = sext i32 %_47 to i64
; CHECK:  %_49 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %_46, i64 0, i64 %_48
; CHECK:  %_50 = load i32, i32* %_4, align 4
; CHECK:  %_51 = sext i32 %_50 to i64
; CHECK:  %_52 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %_49, i64 0, i64 %_51
; CHECK:  %_53 = load i32, i32* %_4, align 4
; CHECK:  %_54 = sub nsw i32 %_53, 1
; CHECK:  %_55 = sext i32 %_54 to i64
; CHECK:  %_56 = getelementptr inbounds [9 x i32], [9 x i32]* %_52, i64 0, i64 %_55
; CHECK:  %_57 = load i32, i32* %_56, align 4
; CHECK:  br label %bb_16
  %_46 = getelementptr inbounds %struct.foo, %struct.foo* %_7, i32 0, i32 2
  %_47 = load i32, i32* %_4, align 4
  %_48 = sext i32 %_47 to i64
  %_49 = getelementptr inbounds [10 x [10 x [9 x i32]]], [10 x [10 x [9 x i32]]]* %_46, i64 0, i64 %_48
  %_50 = load i32, i32* %_4, align 4
  %_51 = sext i32 %_50 to i64
  %_52 = getelementptr inbounds [10 x [9 x i32]], [10 x [9 x i32]]* %_49, i64 0, i64 %_51
  %_53 = load i32, i32* %_4, align 4
  %_54 = sub nsw i32 %_53, 1
  %_55 = sext i32 %_54 to i64
  %_56 = getelementptr inbounds [9 x i32], [9 x i32]* %_52, i64 0, i64 %_55
  %_57 = load i32, i32* %_56, align 4
  %_58 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %_57)
  br label %bb_16

bb_16:                                            ; preds = %bb_15
  %_59 = load i32, i32* %_4, align 4
  %_60 = add nsw i32 %_59, 1
  store i32 %_60, i32* %_4, align 4
  br label %bb_14

bb_17:                                            ; preds = %bb_14
  ret i32 0
}

declare i32 @printf(i8*, ...)
