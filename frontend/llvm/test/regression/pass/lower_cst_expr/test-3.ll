target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

@p = common global [2 x [2 x [2 x i32]]] zeroinitializer, align 16

define void @kalman_global() {
; CHECK-LABEL: @kalman_global(
; CHECK:  %0 = getelementptr inbounds [2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 0, i64 0, i64 0
; CHECK:  store i32 1, i32* %0, align 16
; CHECK:  %1 = getelementptr inbounds [2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 0, i64 0, i64 1
; CHECK:  store i32 1, i32* %1, align 4
; CHECK:  %2 = getelementptr inbounds [2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 0, i64 1, i64 0
; CHECK:  store i32 1, i32* %2, align 8
; CHECK:  %3 = getelementptr inbounds [2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 0, i64 1, i64 1
; CHECK:  store i32 1, i32* %3, align 4
; CHECK:  %4 = getelementptr inbounds [2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 1, i64 0, i64 0
; CHECK:  store i32 1, i32* %4, align 16
; CHECK:  %5 = getelementptr inbounds [2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 1, i64 0, i64 1
; CHECK:  store i32 1, i32* %5, align 4
; CHECK:  %6 = getelementptr inbounds [2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 1, i64 1, i64 0
; CHECK:  store i32 1, i32* %6, align 8
; CHECK:  %7 = getelementptr inbounds [2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 1, i64 1, i64 1
; CHECK:  store i32 1, i32* %7, align 4
; CHECK:  ret void
bb_1:
  store i32 1, i32* getelementptr inbounds ([2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 0, i64 0, i64 0), align 16
  store i32 1, i32* getelementptr inbounds ([2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 0, i64 0, i64 1), align 4
  store i32 1, i32* getelementptr inbounds ([2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 0, i64 1, i64 0), align 8
  store i32 1, i32* getelementptr inbounds ([2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 0, i64 1, i64 1), align 4
  store i32 1, i32* getelementptr inbounds ([2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 1, i64 0, i64 0), align 16
  store i32 1, i32* getelementptr inbounds ([2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 1, i64 0, i64 1), align 4
  store i32 1, i32* getelementptr inbounds ([2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 1, i64 1, i64 0), align 8
  store i32 1, i32* getelementptr inbounds ([2 x [2 x [2 x i32]]], [2 x [2 x [2 x i32]]]* @p, i64 0, i64 1, i64 1, i64 1), align 4
  ret void
}

define i32 @main(i32 %arg_1, i8** %arg_2) {
bb_1:
  %_1 = alloca i32, align 4
  %_2 = alloca i32, align 4
  %_3 = alloca i8**, align 8
  store i32 0, i32* %_1, align 4
  store i32 %arg_1, i32* %_2, align 4
  store i8** %arg_2, i8*** %_3, align 8
  call void @kalman_global()
  ret i32 0
}
