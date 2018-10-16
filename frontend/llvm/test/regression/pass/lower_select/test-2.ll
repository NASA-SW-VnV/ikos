target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"

@buffer = internal global [6 x i8] zeroinitializer, align 1

define internal i8* @my_malloc(i64 %arg_1) {
bb_1:
  %_1 = getelementptr inbounds [6 x i8], [6 x i8]* @buffer, i64 0, i64 0
  ret i8* %_1
}

define i32 @main() local_unnamed_addr {
bb_1:
; CHECK-LABEL: bb_1:
; CHECK:  %_1 = call i32 (...) @__ikos_unknown()
; CHECK:  %_2 = icmp ne i32 %_1, 0
; CHECK:  br i1 %_2, label %bb_1.TrueSelect, label %bb_1.FalseSelect
  %_1 = call i32 (...) @__ikos_unknown() #3
  %_2 = icmp ne i32 %_1, 0
  %malloc.my_malloc = select i1 %_2, i8* (i64)* @malloc, i8* (i64)* @my_malloc
  %_3 = call i8* %malloc.my_malloc(i64 8) #3
  %_4 = icmp eq i8* %_3, null
  br i1 %_4, label %bb_3, label %bb_2

; CHECK: bb_1.TrueSelect:                                  ; preds = %bb_1
; CHECK:   br label %bb_1.AfterSelect

; CHECK: bb_1.FalseSelect:                                 ; preds = %bb_1
; CHECK:   br label %bb_1.AfterSelect

; CHECK: bb_1.AfterSelect:                                 ; preds = %bb_1.FalseSelect, %bb_1.TrueSelect
; CHECK:   %malloc.my_malloc.phi = phi i8* (i64)* [ @malloc, %bb_1.TrueSelect ], [ @my_malloc, %bb_1.FalseSelect ]
; CHECK:   %_3 = call i8* %malloc.my_malloc.phi(i64 8)
; CHECK:   %_4 = icmp eq i8* %_3, null
; CHECK:   br i1 %_4, label %bb_3, label %bb_2

bb_2:                                             ; preds = %bb_1
; CHECK-LABEL: bb_2:                                             ; preds = %bb_1.AfterSelect
; CHECK:   %_5 = bitcast i8* %_3 to i32*
; CHECK:   store i32 1, i32* %_5, align 4
; CHECK:   %_6 = getelementptr inbounds i8, i8* %_3, i64 4
; CHECK:   %_7 = bitcast i8* %_6 to i32*
; CHECK:   store i32 2, i32* %_7, align 4
; CHECK:   br label %bb_3
  %_5 = bitcast i8* %_3 to i32*
  store i32 1, i32* %_5, align 4
  %_6 = getelementptr inbounds i8, i8* %_3, i64 4
  %_7 = bitcast i8* %_6 to i32*
  store i32 2, i32* %_7, align 4
  br label %bb_3

bb_3:                                             ; preds = %bb_2, %bb_1
; CHECK-LABEL: bb_3:                                             ; preds = %bb_2, %bb_1.AfterSelect
; CHECK:   %.0 = phi i32 [ 0, %bb_2 ], [ 1, %bb_1.AfterSelect ]
; CHECK:   ret i32 %.0
  %.0 = phi i32 [ 0, %bb_2 ], [ 1, %bb_1 ]
  ret i32 %.0
}

declare i32 @__ikos_unknown(...) local_unnamed_addr

declare i8* @malloc(i64)
