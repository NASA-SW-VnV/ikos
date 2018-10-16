target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

@main.STRING_MAP = internal constant [6 x i8*] [i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str, i32 0, i32 0), i8* getelementptr inbounds ([23 x i8], [23 x i8]* @.str.1, i32 0, i32 0), i8* getelementptr inbounds ([13 x i8], [13 x i8]* @.str.2, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.3, i32 0, i32 0), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.4, i32 0, i32 0), i8* getelementptr inbounds ([17 x i8], [17 x i8]* @.str.5, i32 0, i32 0)], align 16
@.str = private unnamed_addr constant [10 x i8] c"brown-out\00", align 1
@.str.1 = private unnamed_addr constant [23 x i8] c"configuration mismatch\00", align 1
@.str.2 = private unnamed_addr constant [13 x i8] c"master clear\00", align 1
@.str.3 = private unnamed_addr constant [9 x i8] c"power on\00", align 1
@.str.4 = private unnamed_addr constant [9 x i8] c"software\00", align 1
@.str.5 = private unnamed_addr constant [17 x i8] c"watchdog timeout\00", align 1
@.str.6 = private unnamed_addr constant [8 x i8] c"unknown\00", align 1
@.str.7 = private unnamed_addr constant [33 x i8] c"The cause of the last reset was \00", align 1
@.str.8 = private unnamed_addr constant [22 x i8] c"../../src/init/init.c\00", align 1
@.str.9 = private unnamed_addr constant [1 x i8] zeroinitializer, align 1

define i32 @main(i32 %arg_1, i8** %arg_2) {
bb_1:
  %_1 = alloca i32, align 4
  %_2 = alloca i32, align 4
  %_3 = alloca i8**, align 8
  %_4 = alloca i32, align 4
  %_5 = alloca i8*, align 8
  %_6 = alloca [55 x i8], align 16
  store i32 0, i32* %_1, align 4
  store i32 %arg_1, i32* %_2, align 4
  store i8** %arg_2, i8*** %_3, align 8
  %_7 = call i32 (...) @bsp_reset_get()
  store i32 %_7, i32* %_4, align 4
  %_8 = load i32, i32* %_4, align 4
  %_9 = icmp ule i32 0, %_8
  br i1 %_9, label %bb_2, label %bb_4

bb_2:                                             ; preds = %bb_1
  %_10 = load i32, i32* %_4, align 4
  %_11 = icmp ult i32 %_10, 6
  br i1 %_11, label %bb_3, label %bb_4

bb_3:                                             ; preds = %bb_2
  %_12 = load i32, i32* %_4, align 4
  %_13 = zext i32 %_12 to i64
  %_14 = getelementptr inbounds [6 x i8*], [6 x i8*]* @main.STRING_MAP, i64 0, i64 %_13
  %_15 = load i8*, i8** %_14, align 8
  br label %bb_5

bb_4:                                             ; preds = %bb_2, %bb_1
; CHECK-LABEL: bb_4:
; CHECK:  %0 = getelementptr inbounds [8 x i8], [8 x i8]* @.str.6, i32 0, i32 0
; CHECK:  br label %bb_5
  br label %bb_5

bb_5:                                             ; preds = %bb_4, %bb_3
; CHECK-LABEL: bb_5:
; CHECK:  %_16 = phi i8* [ %_15, %bb_3 ], [ %0, %bb_4 ]
; CHECK:  store i8* %_16, i8** %_5, align 8
; CHECK:  %_17 = getelementptr inbounds [55 x i8], [55 x i8]* %_6, i32 0, i32 0
; CHECK:  %1 = getelementptr inbounds [33 x i8], [33 x i8]* @.str.7, i32 0, i32 0
; CHECK:  %_18 = call i8* @__strcpy_chk(i8* %_17, i8* %1, i64 55)
; CHECK:  %_19 = getelementptr inbounds [55 x i8], [55 x i8]* %_6, i32 0, i32 0
; CHECK:  %_20 = load i8*, i8** %_5, align 8
; CHECK:  %_21 = call i8* @__strcat_chk(i8* %_19, i8* %_20, i64 55)
; CHECK:  %_22 = getelementptr inbounds [55 x i8], [55 x i8]* %_6, i32 0, i32 0
; CHECK:  %2 = getelementptr inbounds [22 x i8], [22 x i8]* @.str.8, i32 0, i32 0
; CHECK:  %3 = getelementptr inbounds [1 x i8], [1 x i8]* @.str.9, i32 0, i32 0
; CHECK:  call void (i32, i8*, i32, i8*, ...) @util_log(i32 1, i8* %2, i32 356, i8* %_22, i8* %3)
; CHECK:  ret i32 0
  %_16 = phi i8* [ %_15, %bb_3 ], [ getelementptr inbounds ([8 x i8], [8 x i8]* @.str.6, i32 0, i32 0), %bb_4 ]
  store i8* %_16, i8** %_5, align 8
  %_17 = getelementptr inbounds [55 x i8], [55 x i8]* %_6, i32 0, i32 0
  %_18 = call i8* @__strcpy_chk(i8* %_17, i8* getelementptr inbounds ([33 x i8], [33 x i8]* @.str.7, i32 0, i32 0), i64 55) #4
  %_19 = getelementptr inbounds [55 x i8], [55 x i8]* %_6, i32 0, i32 0
  %_20 = load i8*, i8** %_5, align 8
  %_21 = call i8* @__strcat_chk(i8* %_19, i8* %_20, i64 55) #4
  %_22 = getelementptr inbounds [55 x i8], [55 x i8]* %_6, i32 0, i32 0
  call void (i32, i8*, i32, i8*, ...) @util_log(i32 1, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.str.8, i32 0, i32 0), i32 356, i8* %_22, i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.str.9, i32 0, i32 0))
  ret i32 0
}

declare i32 @bsp_reset_get(...)

declare i8* @__strcpy_chk(i8*, i8*, i64)

declare i8* @__strcat_chk(i8*, i8*, i64)

declare void @util_log(i32, i8*, i32, i8*, ...)
