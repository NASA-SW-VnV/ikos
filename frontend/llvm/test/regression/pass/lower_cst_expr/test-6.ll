target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

%struct.foo = type { i8, i64, i8, %struct.bar }
%struct.bar = type { i32, i8 }

@x = common global %struct.foo zeroinitializer, align 8

define i32 @main() {
bb_1:
; CHECK-LABEL: bb_1:
; CHECK:  %_1 = alloca i32, align 4
; CHECK:  store i32 0, i32* %_1, align 4
; CHECK:  %0 = getelementptr inbounds %struct.foo, %struct.foo* @x, i32 0, i32 0
; CHECK:  store i8 5, i8* %0, align 8
; CHECK:  %1 = getelementptr inbounds %struct.foo, %struct.foo* @x, i32 0, i32 1
; CHECK:  store i64 2000, i64* %1, align 8
; CHECK:  %2 = getelementptr inbounds %struct.foo, %struct.foo* @x, i32 0, i32 2
; CHECK:  store i8 10, i8* %2, align 8
; CHECK:  %3 = getelementptr inbounds %struct.foo, %struct.foo* @x, i32 0, i32 3, i32 0
; CHECK:  store i32 32, i32* %3, align 4
; CHECK:  %4 = getelementptr inbounds %struct.foo, %struct.foo* @x, i32 0, i32 3, i32 1
; CHECK:  store i8 5, i8* %4, align 4
; CHECK:  %5 = getelementptr inbounds %struct.foo, %struct.foo* @x, i32 0, i32 3, i32 0
; CHECK:  %_2 = load i32, i32* %5, align 4
; CHECK:  %_3 = icmp sgt i32 %_2, 0
; CHECK:  br i1 %_3, label %bb_2, label %bb_3
  %_1 = alloca i32, align 4
  store i32 0, i32* %_1, align 4
  store i8 5, i8* getelementptr inbounds (%struct.foo, %struct.foo* @x, i32 0, i32 0), align 8
  store i64 2000, i64* getelementptr inbounds (%struct.foo, %struct.foo* @x, i32 0, i32 1), align 8
  store i8 10, i8* getelementptr inbounds (%struct.foo, %struct.foo* @x, i32 0, i32 2), align 8
  store i32 32, i32* getelementptr inbounds (%struct.foo, %struct.foo* @x, i32 0, i32 3, i32 0), align 4
  store i8 5, i8* getelementptr inbounds (%struct.foo, %struct.foo* @x, i32 0, i32 3, i32 1), align 4
  %_2 = load i32, i32* getelementptr inbounds (%struct.foo, %struct.foo* @x, i32 0, i32 3, i32 0), align 4
  %_3 = icmp sgt i32 %_2, 0
  br i1 %_3, label %bb_2, label %bb_3

bb_2:                                             ; preds = %bb_1
; CHECK-LABEL: bb_2:
; CHECK:  %6 = getelementptr inbounds %struct.foo, %struct.foo* @x, i32 0, i32 3, i32 0
; CHECK:  %_4 = load i32, i32* %6, align 4
; CHECK:  %7 = getelementptr inbounds %struct.foo, %struct.foo* @x, i32 0, i32 2
; CHECK:  %_5 = load i8, i8* %7, align 8
; CHECK:  %_6 = zext i8 %_5 to i32
; CHECK:  %_7 = add nsw i32 %_4, %_6
; CHECK:  %_8 = icmp eq i32 %_7, 42
; CHECK:  %_9 = zext i1 %_8 to i32
; CHECK:  call void @__ikos_assert(i32 %_9)
; CHECK:  br label %bb_3
  %_4 = load i32, i32* getelementptr inbounds (%struct.foo, %struct.foo* @x, i32 0, i32 3, i32 0), align 4
  %_5 = load i8, i8* getelementptr inbounds (%struct.foo, %struct.foo* @x, i32 0, i32 2), align 8
  %_6 = zext i8 %_5 to i32
  %_7 = add nsw i32 %_4, %_6
  %_8 = icmp eq i32 %_7, 42
  %_9 = zext i1 %_8 to i32
  call void @__ikos_assert(i32 %_9)
  br label %bb_3

bb_3:                                             ; preds = %bb_2, %bb_1
  ret i32 42
}

declare void @__ikos_assert(i32)
