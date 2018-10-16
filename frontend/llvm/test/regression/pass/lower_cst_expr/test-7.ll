target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

%union.anon = type { %struct.anon.0 }
%struct.anon.0 = type { i16, i16 }
%struct.anon = type { i8, i8, i8, i8 }

@regs = internal global %union.anon zeroinitializer, align 2

define i32 @main() {
bb_1:
; CHECK-LABEL: bb_1:
; CHECK:  %_1 = alloca i32, align 4
; CHECK:  store i32 0, i32* %_1, align 4
; CHECK:  %0 = getelementptr inbounds %union.anon, %union.anon* @regs, i32 0, i32 0, i32 0
; CHECK:  store i16 5, i16* %0, align 2
; CHECK:  %1 = bitcast %union.anon* @regs to %struct.anon*
; CHECK:  %2 = getelementptr inbounds %struct.anon, %struct.anon* %1, i32 0, i32 1
; CHECK:  %_2 = load i8, i8* %2, align 1
; CHECK:  %_3 = icmp ne i8 %_2, 0
; CHECK:  br i1 %_3, label %bb_3, label %bb_2
  %_1 = alloca i32, align 4
  store i32 0, i32* %_1, align 4
  store i16 5, i16* getelementptr inbounds (%union.anon, %union.anon* @regs, i32 0, i32 0, i32 0), align 2
  %_2 = load i8, i8* getelementptr inbounds (%struct.anon, %struct.anon* bitcast (%union.anon* @regs to %struct.anon*), i32 0, i32 1), align 1
  %_3 = icmp ne i8 %_2, 0
  br i1 %_3, label %bb_3, label %bb_2

bb_2:                                             ; preds = %bb_1
; CHECK-LABEL: bb_2:
; CHECK:  %3 = bitcast %union.anon* @regs to %struct.anon*
; CHECK:  %4 = getelementptr inbounds %struct.anon, %struct.anon* %3, i32 0, i32 0
; CHECK:  %_4 = load i8, i8* %4, align 2
; CHECK:  %5 = bitcast %union.anon* @regs to %struct.anon*
; CHECK:  %6 = getelementptr inbounds %struct.anon, %struct.anon* %5, i32 0, i32 2
; CHECK:  store i8 %_4, i8* %6, align 2
; CHECK:  br label %bb_4
  %_4 = load i8, i8* getelementptr inbounds (%struct.anon, %struct.anon* bitcast (%union.anon* @regs to %struct.anon*), i32 0, i32 0), align 2
  store i8 %_4, i8* getelementptr inbounds (%struct.anon, %struct.anon* bitcast (%union.anon* @regs to %struct.anon*), i32 0, i32 2), align 2
  br label %bb_4

bb_3:                                             ; preds = %bb_1
; CHECK-LABEL: bb_3:
; CHECK:  %7 = bitcast %union.anon* @regs to %struct.anon*
; CHECK:  %8 = getelementptr inbounds %struct.anon, %struct.anon* %7, i32 0, i32 0
; CHECK:  %_5 = load i8, i8* %8, align 2
; CHECK:  %9 = bitcast %union.anon* @regs to %struct.anon*
; CHECK:  %10 = getelementptr inbounds %struct.anon, %struct.anon* %9, i32 0, i32 3
; CHECK:  store i8 %_5, i8* %10, align 1
; CHECK:  br label %bb_4
  %_5 = load i8, i8* getelementptr inbounds (%struct.anon, %struct.anon* bitcast (%union.anon* @regs to %struct.anon*), i32 0, i32 0), align 2
  store i8 %_5, i8* getelementptr inbounds (%struct.anon, %struct.anon* bitcast (%union.anon* @regs to %struct.anon*), i32 0, i32 3), align 1
  br label %bb_4

bb_4:                                             ; preds = %bb_3, %bb_2
; CHECK-LABEL: bb_4:
; CHECK:  %11 = getelementptr inbounds %union.anon, %union.anon* @regs, i32 0, i32 0, i32 0
; CHECK:  %_6 = load i16, i16* %11, align 2
; CHECK:  %_7 = zext i16 %_6 to i32
; CHECK:  %_8 = icmp eq i32 %_7, 5
; CHECK:  %_9 = zext i1 %_8 to i32
; CHECK:  call void @__ikos_assert(i32 %_9)
; CHECK:  %12 = getelementptr inbounds %union.anon, %union.anon* @regs, i32 0, i32 0, i32 1
; CHECK:  %_10 = load i16, i16* %12, align 2
; CHECK:  %_11 = zext i16 %_10 to i32
; CHECK:  %_12 = icmp eq i32 %_11, 5
; CHECK:  %_13 = zext i1 %_12 to i32
; CHECK:  call void @__ikos_assert(i32 %_13)
; CHECK:  ret i32 0
  %_6 = load i16, i16* getelementptr inbounds (%union.anon, %union.anon* @regs, i32 0, i32 0, i32 0), align 2
  %_7 = zext i16 %_6 to i32
  %_8 = icmp eq i32 %_7, 5
  %_9 = zext i1 %_8 to i32
  call void @__ikos_assert(i32 %_9)
  %_10 = load i16, i16* getelementptr inbounds (%union.anon, %union.anon* @regs, i32 0, i32 0, i32 1), align 2
  %_11 = zext i16 %_10 to i32
  %_12 = icmp eq i32 %_11, 5
  %_13 = zext i1 %_12 to i32
  call void @__ikos_assert(i32 %_13)
  ret i32 0
}

declare void @__ikos_assert(i32)
