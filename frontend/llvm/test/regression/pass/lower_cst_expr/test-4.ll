target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

%struct.__va_list_tag = type { i32, i32, i8*, i8* }

@.str = private unnamed_addr constant [11 x i8] c"string %s\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"int %d\0A\00", align 1
@.str.2 = private unnamed_addr constant [9 x i8] c"char %c\0A\00", align 1
@.str.3 = private unnamed_addr constant [11 x i8] c"double %f\0A\00", align 1
@.str.4 = private unnamed_addr constant [5 x i8] c"sdcd\00", align 1
@.str.5 = private unnamed_addr constant [10 x i8] c"my string\00", align 1

; Function Attrs: noinline nounwind ssp uwtable
define void @foo(i8* %arg_1, ...) #0 {
bb_1:
  %_1 = alloca i8*, align 8
  %_2 = alloca [1 x %struct.__va_list_tag], align 16
  %_3 = alloca i32, align 4
  %_4 = alloca i8, align 1
  %_5 = alloca i8*, align 8
  %_6 = alloca double, align 8
  store i8* %arg_1, i8** %_1, align 8
  %_7 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %_2, i32 0, i32 0
  %_8 = bitcast %struct.__va_list_tag* %_7 to i8*
  call void @llvm.va_start(i8* %_8)
  br label %bb_2

bb_2:                                             ; preds = %bb_21, %bb_1
  %_9 = load i8*, i8** %_1, align 8
  %_10 = load i8, i8* %_9, align 1
  %_11 = icmp ne i8 %_10, 0
  br i1 %_11, label %bb_3, label %bb_22

bb_3:                                             ; preds = %bb_2
  %_12 = load i8*, i8** %_1, align 8
  %_13 = getelementptr inbounds i8, i8* %_12, i32 1
  store i8* %_13, i8** %_1, align 8
  %_14 = load i8, i8* %_12, align 1
  %_15 = sext i8 %_14 to i32
  switch i32 %_15, label %bb_20 [
    i32 115, label %bb_4
    i32 100, label %bb_8
    i32 99, label %bb_12
    i32 102, label %bb_16
  ]

bb_4:                                             ; preds = %bb_3
  %_16 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %_2, i32 0, i32 0
  %_17 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_16, i32 0, i32 0
  %_18 = load i32, i32* %_17, align 16
  %_19 = icmp ule i32 %_18, 40
  br i1 %_19, label %bb_5, label %bb_6

bb_5:                                             ; preds = %bb_4
  %_20 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_16, i32 0, i32 3
  %_21 = load i8*, i8** %_20, align 16
  %_22 = getelementptr i8, i8* %_21, i32 %_18
  %_23 = bitcast i8* %_22 to i8**
  %_24 = add i32 %_18, 8
  store i32 %_24, i32* %_17, align 16
  br label %bb_7

bb_6:                                             ; preds = %bb_4
  %_25 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_16, i32 0, i32 2
  %_26 = load i8*, i8** %_25, align 8
  %_27 = bitcast i8* %_26 to i8**
  %_28 = getelementptr i8, i8* %_26, i32 8
  store i8* %_28, i8** %_25, align 8
  br label %bb_7

bb_7:                                             ; preds = %bb_6, %bb_5
; CHECK-LABEL: bb_7:
; CHECK:  %_29 = phi i8** [ %_23, %bb_5 ], [ %_27, %bb_6 ]
; CHECK:  %_30 = load i8*, i8** %_29, align 8
; CHECK:  store i8* %_30, i8** %_5, align 8
; CHECK:  %_31 = load i8*, i8** %_5, align 8
; CHECK:  %0 = getelementptr inbounds [11 x i8], [11 x i8]* @.str, i32 0, i32 0
; CHECK:  %_32 = call i32 (i8*, ...) @printf(i8* %0, i8* %_31)
; CHECK:  br label %bb_21
  %_29 = phi i8** [ %_23, %bb_5 ], [ %_27, %bb_6 ]
  %_30 = load i8*, i8** %_29, align 8
  store i8* %_30, i8** %_5, align 8
  %_31 = load i8*, i8** %_5, align 8
  %_32 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str, i32 0, i32 0), i8* %_31)
  br label %bb_21

bb_8:                                             ; preds = %bb_3
  %_33 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %_2, i32 0, i32 0
  %_34 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_33, i32 0, i32 0
  %_35 = load i32, i32* %_34, align 16
  %_36 = icmp ule i32 %_35, 40
  br i1 %_36, label %bb_9, label %bb_10

bb_9:                                             ; preds = %bb_8
  %_37 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_33, i32 0, i32 3
  %_38 = load i8*, i8** %_37, align 16
  %_39 = getelementptr i8, i8* %_38, i32 %_35
  %_40 = bitcast i8* %_39 to i32*
  %_41 = add i32 %_35, 8
  store i32 %_41, i32* %_34, align 16
  br label %bb_11

bb_10:                                            ; preds = %bb_8
  %_42 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_33, i32 0, i32 2
  %_43 = load i8*, i8** %_42, align 8
  %_44 = bitcast i8* %_43 to i32*
  %_45 = getelementptr i8, i8* %_43, i32 8
  store i8* %_45, i8** %_42, align 8
  br label %bb_11

bb_11:                                            ; preds = %bb_10, %bb_9
; CHECK-LABEL: bb_11:
; CHECK:  %_46 = phi i32* [ %_40, %bb_9 ], [ %_44, %bb_10 ]
; CHECK:  %_47 = load i32, i32* %_46, align 4
; CHECK:  store i32 %_47, i32* %_3, align 4
; CHECK:  %_48 = load i32, i32* %_3, align 4
; CHECK:  %1 = getelementptr inbounds [8 x i8], [8 x i8]* @.str.1, i32 0, i32 0
; CHECK:  %_49 = call i32 (i8*, ...) @printf(i8* %1, i32 %_48)
; CHECK:  br label %bb_21
  %_46 = phi i32* [ %_40, %bb_9 ], [ %_44, %bb_10 ]
  %_47 = load i32, i32* %_46, align 4
  store i32 %_47, i32* %_3, align 4
  %_48 = load i32, i32* %_3, align 4
  %_49 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0), i32 %_48)
  br label %bb_21

bb_12:                                            ; preds = %bb_3
  %_50 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %_2, i32 0, i32 0
  %_51 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_50, i32 0, i32 0
  %_52 = load i32, i32* %_51, align 16
  %_53 = icmp ule i32 %_52, 40
  br i1 %_53, label %bb_13, label %bb_14

bb_13:                                            ; preds = %bb_12
  %_54 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_50, i32 0, i32 3
  %_55 = load i8*, i8** %_54, align 16
  %_56 = getelementptr i8, i8* %_55, i32 %_52
  %_57 = bitcast i8* %_56 to i32*
  %_58 = add i32 %_52, 8
  store i32 %_58, i32* %_51, align 16
  br label %bb_15

bb_14:                                            ; preds = %bb_12
  %_59 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_50, i32 0, i32 2
  %_60 = load i8*, i8** %_59, align 8
  %_61 = bitcast i8* %_60 to i32*
  %_62 = getelementptr i8, i8* %_60, i32 8
  store i8* %_62, i8** %_59, align 8
  br label %bb_15

bb_15:                                            ; preds = %bb_14, %bb_13
; CHECK-LABEL: bb_15:
; CHECK:  %_63 = phi i32* [ %_57, %bb_13 ], [ %_61, %bb_14 ]
; CHECK:  %_64 = load i32, i32* %_63, align 4
; CHECK:  %_65 = trunc i32 %_64 to i8
; CHECK:  store i8 %_65, i8* %_4, align 1
; CHECK:  %_66 = load i8, i8* %_4, align 1
; CHECK:  %_67 = sext i8 %_66 to i32
; CHECK:  %2 = getelementptr inbounds [9 x i8], [9 x i8]* @.str.2, i32 0, i32 0
; CHECK:  %_68 = call i32 (i8*, ...) @printf(i8* %2, i32 %_67)
; CHECK:  br label %bb_21
  %_63 = phi i32* [ %_57, %bb_13 ], [ %_61, %bb_14 ]
  %_64 = load i32, i32* %_63, align 4
  %_65 = trunc i32 %_64 to i8
  store i8 %_65, i8* %_4, align 1
  %_66 = load i8, i8* %_4, align 1
  %_67 = sext i8 %_66 to i32
  %_68 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str.2, i32 0, i32 0), i32 %_67)
  br label %bb_21

bb_16:                                            ; preds = %bb_3
  %_69 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %_2, i32 0, i32 0
  %_70 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_69, i32 0, i32 1
  %_71 = load i32, i32* %_70, align 4
  %_72 = icmp ule i32 %_71, 160
  br i1 %_72, label %bb_17, label %bb_18

bb_17:                                            ; preds = %bb_16
  %_73 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_69, i32 0, i32 3
  %_74 = load i8*, i8** %_73, align 16
  %_75 = getelementptr i8, i8* %_74, i32 %_71
  %_76 = bitcast i8* %_75 to double*
  %_77 = add i32 %_71, 16
  store i32 %_77, i32* %_70, align 4
  br label %bb_19

bb_18:                                            ; preds = %bb_16
  %_78 = getelementptr inbounds %struct.__va_list_tag, %struct.__va_list_tag* %_69, i32 0, i32 2
  %_79 = load i8*, i8** %_78, align 8
  %_80 = bitcast i8* %_79 to double*
  %_81 = getelementptr i8, i8* %_79, i32 8
  store i8* %_81, i8** %_78, align 8
  br label %bb_19

bb_19:                                            ; preds = %bb_18, %bb_17
; CHECK-LABEL: bb_19:
; CHECK:  %_82 = phi double* [ %_76, %bb_17 ], [ %_80, %bb_18 ]
; CHECK:  %_83 = load double, double* %_82, align 8
; CHECK:  store double %_83, double* %_6, align 8
; CHECK:  %_84 = load double, double* %_6, align 8
; CHECK:  %3 = getelementptr inbounds [11 x i8], [11 x i8]* @.str.3, i32 0, i32 0
; CHECK:  %_85 = call i32 (i8*, ...) @printf(i8* %3, double %_84)
; CHECK:  br label %bb_21
  %_82 = phi double* [ %_76, %bb_17 ], [ %_80, %bb_18 ]
  %_83 = load double, double* %_82, align 8
  store double %_83, double* %_6, align 8
  %_84 = load double, double* %_6, align 8
  %_85 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([11 x i8], [11 x i8]* @.str.3, i32 0, i32 0), double %_84)
  br label %bb_21

bb_20:                                            ; preds = %bb_3
  br label %bb_21

bb_21:                                            ; preds = %bb_20, %bb_19, %bb_15, %bb_11, %bb_7
  br label %bb_2

bb_22:                                            ; preds = %bb_2
  %_86 = getelementptr inbounds [1 x %struct.__va_list_tag], [1 x %struct.__va_list_tag]* %_2, i32 0, i32 0
  %_87 = bitcast %struct.__va_list_tag* %_86 to i8*
  call void @llvm.va_end(i8* %_87)
  ret void
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind
declare void @llvm.va_start(i8*) #2

declare i32 @printf(i8*, ...) #3

; Function Attrs: nounwind
declare void @llvm.va_end(i8*) #2

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 {
bb_1:
; CHECK-LABEL: @main(
; CHECK:  %_1 = alloca i32, align 4
; CHECK:  store i32 0, i32* %_1, align 4
; CHECK:  %0 = getelementptr inbounds [5 x i8], [5 x i8]* @.str.4, i32 0, i32 0
; CHECK:  %1 = getelementptr inbounds [10 x i8], [10 x i8]* @.str.5, i32 0, i32 0
; CHECK:  call void (i8*, ...) @foo(i8* %0, i8* %1, i32 1, i32 97, i32 3, double 1.123000e+00)
; CHECK:  ret i32 0
  %_1 = alloca i32, align 4
  store i32 0, i32* %_1, align 4
  call void (i8*, ...) @foo(i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.4, i32 0, i32 0), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @.str.5, i32 0, i32 0), i32 1, i32 97, i32 3, double 1.123000e+00)
  ret i32 0
}

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core2" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
