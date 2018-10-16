target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main(i32 %arg_1, i8** %arg_2) {
bb_1:
  %_1 = alloca i32, align 4
  %_2 = alloca i32, align 4
  %_3 = alloca i8**, align 8
  %_4 = alloca i32, align 4
  %_5 = alloca [10 x i32], align 16
  store i32 0, i32* %_1, align 4
  store i32 %arg_1, i32* %_2, align 4
  store i8** %arg_2, i8*** %_3, align 8
  store i32 0, i32* %_4, align 4
  br label %bb_2

bb_2:                                             ; preds = %bb_4, %bb_1
  %_6 = load i32, i32* %_4, align 4
  %_7 = icmp slt i32 %_6, 10
  br i1 %_7, label %bb_3, label %bb_5

bb_3:                                             ; preds = %bb_2
  %_8 = load i32, i32* %_4, align 4
  %_9 = load i32, i32* %_4, align 4
  %_10 = sext i32 %_9 to i64
  %_11 = getelementptr inbounds [10 x i32], [10 x i32]* %_5, i64 0, i64 %_10
  store i32 %_8, i32* %_11, align 4
  br label %bb_4

bb_4:                                             ; preds = %bb_3
  %_12 = load i32, i32* %_4, align 4
  %_13 = add nsw i32 %_12, 1
  store i32 %_13, i32* %_4, align 4
  br label %bb_2

bb_5:                                             ; preds = %bb_2
; CHECK-LABEL: bb_5:
; CHECK:  %_14 = load i32, i32* %_4, align 4
; CHECK:  %_15 = sub nsw i32 %_14, 1
; CHECK:  %_16 = sext i32 %_15 to i64
; CHECK:  %_17 = getelementptr inbounds [10 x i32], [10 x i32]* %_5, i64 0, i64 %_16
; CHECK:  %_18 = load i32, i32* %_17, align 4
; CHECK:  ret i32 0
  %_14 = load i32, i32* %_4, align 4
  %_15 = sub nsw i32 %_14, 1
  %_16 = sext i32 %_15 to i64
  %_17 = getelementptr inbounds [10 x i32], [10 x i32]* %_5, i64 0, i64 %_16
  %_18 = load i32, i32* %_17, align 4
  %_19 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %_18)
  ret i32 0
}

declare i32 @printf(i8*, ...)
