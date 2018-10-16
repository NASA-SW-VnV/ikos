target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define i32 @main(i32 %arg_1, i8** %arg_2) {
bb_1:
  %_1 = alloca i32, align 4
  %_2 = alloca i32, align 4
  %_3 = alloca i8**, align 8
  %_4 = alloca i32*, align 8
  %_5 = alloca i32, align 4
  store i32 0, i32* %_1, align 4
  store i32 %arg_1, i32* %_2, align 4
  store i8** %arg_2, i8*** %_3, align 8
  %_6 = call i8* @malloc(i64 40)
  %_7 = bitcast i8* %_6 to i32*
  store i32* %_7, i32** %_4, align 8
  %_8 = load i32*, i32** %_4, align 8
  %_9 = icmp ne i32* %_8, null
  br i1 %_9, label %bb_3, label %bb_2

bb_2:                                             ; preds = %bb_1
  store i32 1, i32* %_1, align 4
  br label %bb_8

bb_3:                                             ; preds = %bb_1
  store i32 9, i32* %_5, align 4
  br label %bb_4

bb_4:                                             ; preds = %bb_6, %bb_3
  %_10 = load i32, i32* %_5, align 4
  %_11 = icmp sge i32 %_10, 0
  br i1 %_11, label %bb_5, label %bb_7

bb_5:                                             ; preds = %bb_4
  %_12 = load i32, i32* %_5, align 4
  %_13 = load i32*, i32** %_4, align 8
  %_14 = load i32, i32* %_5, align 4
  %_15 = sext i32 %_14 to i64
  %_16 = getelementptr inbounds i32, i32* %_13, i64 %_15
  store i32 %_12, i32* %_16, align 4
  br label %bb_6

bb_6:                                             ; preds = %bb_5
  %_17 = load i32, i32* %_5, align 4
  %_18 = add nsw i32 %_17, -1
  store i32 %_18, i32* %_5, align 4
  br label %bb_4

bb_7:                                             ; preds = %bb_4
; CHECK-LABEL: bb_7:
; CHECK:  %_19 = load i32*, i32** %_4, align 8
; CHECK:  %_20 = getelementptr inbounds i32, i32* %_19, i64 9
; CHECK:  %_21 = load i32, i32* %_20, align 4
; CHECK:  store i32 0, i32* %_1, align 4
; CHECK:  br label %bb_8
  %_19 = load i32*, i32** %_4, align 8
  %_20 = getelementptr inbounds i32, i32* %_19, i64 9
  %_21 = load i32, i32* %_20, align 4
  %_22 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str, i32 0, i32 0), i32 %_21)
  store i32 0, i32* %_1, align 4
  br label %bb_8

bb_8:                                             ; preds = %bb_7, %bb_2
  %_23 = load i32, i32* %_1, align 4
  ret i32 %_23
}

declare i8* @malloc(i64)

declare i32 @printf(i8*, ...)
