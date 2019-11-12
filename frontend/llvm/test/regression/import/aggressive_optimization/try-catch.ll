; ModuleID = 'try-catch.pp.bc'
source_filename = "try-catch.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%class.A = type { i32 (...)** }
%class.B = type { %class.A }
%class.C = type { %class.B }

@_ZTI1A = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1A, i32 0, i32 0) }, align 8
; CHECK: define {0: si8*, 8: si8*}* @_ZTI1A, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv117__class_type_infoE, 8 * 2
; CHECK:   si8* %2 = ptrshift @_ZTS1A, 3 * 0, 1 * 0
; CHECK:   si8* %3 = bitcast %1
; CHECK:   store @_ZTI1A, {0: %3, 8: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTI1B = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1B, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*) }, align 8
; CHECK: define {0: si8*, 8: si8*, 16: si8*}* @_ZTI1B, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv120__si_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1A
; CHECK:   si8* %3 = ptrshift @_ZTS1B, 3 * 0, 1 * 0
; CHECK:   si8* %4 = bitcast %1
; CHECK:   store @_ZTI1B, {0: %4, 8: %3, 16: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTI1C = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1C, i32 0, i32 0), i8* bitcast ({ i8*, i8*, i8* }* @_ZTI1B to i8*) }, align 8
; CHECK: define {0: si8*, 8: si8*, 16: si8*}* @_ZTI1C, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv120__si_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1B
; CHECK:   si8* %3 = ptrshift @_ZTS1C, 3 * 0, 1 * 0
; CHECK:   si8* %4 = bitcast %1
; CHECK:   store @_ZTI1C, {0: %4, 8: %3, 16: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTS1A = internal constant [3 x i8] c"1A\00", align 1
; CHECK: define [3 x si8]* @_ZTS1A, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1A, [49, 65, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1B = internal constant [3 x i8] c"1B\00", align 1
; CHECK: define [3 x si8]* @_ZTS1B, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1B, [49, 66, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1C = internal constant [3 x i8] c"1C\00", align 1
; CHECK: define [3 x si8]* @_ZTS1C, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1C, [49, 67, 0], align 1
; CHECK: }
; CHECK: }

@_ZTV1A = internal unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i8* bitcast (void (%class.A*, i32)* @_ZN1A1fEi to i8*), i8* bitcast (i32 (%class.A*)* @_ZN1A1gEv to i8*)] }, align 8
; CHECK: define {0: [4 x si8*]}* @_ZTV1A, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZN1A1gEv
; CHECK:   si8* %2 = bitcast @_ZN1A1fEi
; CHECK:   si8* %3 = bitcast @_ZTI1A
; CHECK:   store @_ZTV1A, {0: [null, %3, %2, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTV1B = internal unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI1B to i8*), i8* bitcast (void (%class.B*, i32)* @_ZN1B1fEi to i8*), i8* bitcast (i32 (%class.B*)* @_ZN1B1gEv to i8*)] }, align 8
; CHECK: define {0: [4 x si8*]}* @_ZTV1B, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZN1B1gEv
; CHECK:   si8* %2 = bitcast @_ZN1B1fEi
; CHECK:   si8* %3 = bitcast @_ZTI1B
; CHECK:   store @_ZTV1B, {0: [null, %3, %2, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTV1C = internal unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI1C to i8*), i8* bitcast (void (%class.C*, i32)* @_ZN1C1fEi to i8*), i8* bitcast (i32 (%class.C*)* @_ZN1C1gEv to i8*)] }, align 8
; CHECK: define {0: [4 x si8*]}* @_ZTV1C, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZN1C1gEv
; CHECK:   si8* %2 = bitcast @_ZN1C1fEi
; CHECK:   si8* %3 = bitcast @_ZTI1C
; CHECK:   store @_ZTV1C, {0: [null, %3, %2, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
; CHECK: declare si8** @_ZTVN10__cxxabiv117__class_type_infoE

@_ZTVN10__cxxabiv120__si_class_type_infoE = external global i8*
; CHECK: declare si8** @_ZTVN10__cxxabiv120__si_class_type_infoE

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_Z1hi(i32) unnamed_addr #0 !dbg !12 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !15, metadata !DIExpression()), !dbg !16
  ret void, !dbg !17
}
; CHECK: define void @_Z1hi(si32 %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc i32 @_Z2hhi(i32) unnamed_addr #0 !dbg !18 {
  call void @llvm.dbg.value(metadata i32 %0, metadata !21, metadata !DIExpression()), !dbg !22
  %2 = mul nsw i32 %0, %0, !dbg !23
  ret i32 %2, !dbg !24
}
; CHECK: define si32 @_Z2hhi(si32 %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 %2 = %1 smul.nw %1
; CHECK:   return %2
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define internal fastcc void @_Z3runP1A(%class.A*) unnamed_addr #1 !dbg !25 {
  call void @llvm.dbg.value(metadata %class.A* %0, metadata !43, metadata !DIExpression()), !dbg !44
  %2 = bitcast %class.A* %0 to void (%class.A*, i32)***, !dbg !45
  %3 = load void (%class.A*, i32)**, void (%class.A*, i32)*** %2, align 8, !dbg !45
  %4 = load void (%class.A*, i32)*, void (%class.A*, i32)** %3, align 8, !dbg !45
  call void %4(%class.A* %0, i32 12), !dbg !45
  %5 = bitcast %class.A* %0 to i32 (%class.A*)***, !dbg !46
  %6 = load i32 (%class.A*)**, i32 (%class.A*)*** %5, align 8, !dbg !46
  %7 = getelementptr inbounds i32 (%class.A*)*, i32 (%class.A*)** %6, i64 1, !dbg !46
  %8 = load i32 (%class.A*)*, i32 (%class.A*)** %7, align 8, !dbg !46
  %9 = call i32 %8(%class.A* %0), !dbg !46
  call void @llvm.dbg.value(metadata i32 %9, metadata !47, metadata !DIExpression()), !dbg !44
  call fastcc void @_Z1hi(i32 14), !dbg !48
  %10 = call fastcc i32 @_Z2hhi(i32 15), !dbg !49
  call void @llvm.dbg.value(metadata i32 %10, metadata !47, metadata !DIExpression()), !dbg !44
  ret void, !dbg !50
}
; CHECK: define void @_Z3runP1A({0: si32 (...)**}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   void ({0: si32 (...)**}*, si32)*** %2 = bitcast %1
; CHECK:   void ({0: si32 (...)**}*, si32)** %3 = load %2, align 8
; CHECK:   void ({0: si32 (...)**}*, si32)* %4 = load %3, align 8
; CHECK:   call %4(%1, 12)
; CHECK:   si32 ({0: si32 (...)**}*)*** %5 = bitcast %1
; CHECK:   si32 ({0: si32 (...)**}*)** %6 = load %5, align 8
; CHECK:   si32 ({0: si32 (...)**}*)** %7 = ptrshift %6, 8 * 1
; CHECK:   si32 ({0: si32 (...)**}*)* %8 = load %7, align 8
; CHECK:   si32 %9 = call %8(%1)
; CHECK:   call @_Z1hi(14)
; CHECK:   si32 %10 = call @_Z2hhi(15)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal void @_ZN1A1fEi(%class.A*, i32) unnamed_addr #0 align 2 !dbg !119 {
  call void @llvm.dbg.value(metadata %class.A* %0, metadata !120, metadata !DIExpression()), !dbg !121
  call void @llvm.dbg.value(metadata i32 %1, metadata !122, metadata !DIExpression()), !dbg !121
  ret void, !dbg !123
}
; CHECK: define void @_ZN1A1fEi({0: si32 (...)**}* %1, si32 %2) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal i32 @_ZN1A1gEv(%class.A*) unnamed_addr #0 align 2 !dbg !124 {
  call void @llvm.dbg.value(metadata %class.A* %0, metadata !125, metadata !DIExpression()), !dbg !126
  ret i32 0, !dbg !127
}
; CHECK: define si32 @_ZN1A1gEv({0: si32 (...)**}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1AC2Ev(%class.A*) unnamed_addr #0 align 2 !dbg !103 {
  call void @llvm.dbg.value(metadata %class.A* %0, metadata !107, metadata !DIExpression()), !dbg !108
  %2 = getelementptr inbounds %class.A, %class.A* %0, i64 0, i32 0, !dbg !109
  %3 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1A, i64 0, i32 0, i64 2, !dbg !109
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !109
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !109
  ret void, !dbg !109
}
; CHECK: define void @_ZN1AC2Ev({0: si32 (...)**}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = ptrshift %1, 8 * 0, 1 * 0
; CHECK:   si8** %3 = ptrshift @_ZTV1A, 32 * 0, 1 * 0, 8 * 2
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal void @_ZN1B1fEi(%class.B*, i32) unnamed_addr #0 align 2 !dbg !110 {
  call void @llvm.dbg.value(metadata %class.B* %0, metadata !111, metadata !DIExpression()), !dbg !112
  call void @llvm.dbg.value(metadata i32 %1, metadata !113, metadata !DIExpression()), !dbg !112
  ret void, !dbg !114
}
; CHECK: define void @_ZN1B1fEi({0: {0: si32 (...)**}}* %1, si32 %2) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal i32 @_ZN1B1gEv(%class.B*) unnamed_addr #0 align 2 !dbg !115 {
  call void @llvm.dbg.value(metadata %class.B* %0, metadata !116, metadata !DIExpression()), !dbg !117
  ret i32 0, !dbg !118
}
; CHECK: define si32 @_ZN1B1gEv({0: {0: si32 (...)**}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1BC1Ev(%class.B*) unnamed_addr #0 align 2 !dbg !83 {
  call void @llvm.dbg.value(metadata %class.B* %0, metadata !87, metadata !DIExpression()), !dbg !89
  call fastcc void @_ZN1BC2Ev(%class.B* %0) #4, !dbg !90
  ret void, !dbg !90
}
; CHECK: define void @_ZN1BC1Ev({0: {0: si32 (...)**}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN1BC2Ev(%1)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1BC2Ev(%class.B*) unnamed_addr #0 align 2 !dbg !99 {
  call void @llvm.dbg.value(metadata %class.B* %0, metadata !100, metadata !DIExpression()), !dbg !101
  %2 = getelementptr inbounds %class.B, %class.B* %0, i64 0, i32 0, !dbg !102
  call fastcc void @_ZN1AC2Ev(%class.A* %2) #4, !dbg !102
  %3 = getelementptr inbounds %class.B, %class.B* %0, i64 0, i32 0, i32 0, !dbg !102
  %4 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1B, i64 0, i32 0, i64 2, !dbg !102
  %5 = bitcast i8** %4 to i32 (...)**, !dbg !102
  store i32 (...)** %5, i32 (...)*** %3, align 8, !dbg !102
  ret void, !dbg !102
}
; CHECK: define void @_ZN1BC2Ev({0: {0: si32 (...)**}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si32 (...)**}* %2 = ptrshift %1, 8 * 0, 1 * 0
; CHECK:   call @_ZN1AC2Ev(%2)
; CHECK:   si32 (...)*** %3 = ptrshift %1, 8 * 0, 1 * 0, 1 * 0
; CHECK:   si8** %4 = ptrshift @_ZTV1B, 32 * 0, 1 * 0, 8 * 2
; CHECK:   si32 (...)** %5 = bitcast %4
; CHECK:   store %3, %5, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal void @_ZN1C1fEi(%class.C*, i32) unnamed_addr #0 align 2 !dbg !132 {
  call void @llvm.dbg.value(metadata %class.C* %0, metadata !133, metadata !DIExpression()), !dbg !134
  call void @llvm.dbg.value(metadata i32 %1, metadata !135, metadata !DIExpression()), !dbg !134
  ret void, !dbg !136
}
; CHECK: define void @_ZN1C1fEi({0: {0: {0: si32 (...)**}}}* %1, si32 %2) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal i32 @_ZN1C1gEv(%class.C*) unnamed_addr #0 align 2 !dbg !137 {
  call void @llvm.dbg.value(metadata %class.C* %0, metadata !138, metadata !DIExpression()), !dbg !139
  ret i32 1, !dbg !140
}
; CHECK: define si32 @_ZN1C1gEv({0: {0: {0: si32 (...)**}}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1CC1Ev(%class.C*) unnamed_addr #0 align 2 !dbg !91 {
  call void @llvm.dbg.value(metadata %class.C* %0, metadata !95, metadata !DIExpression()), !dbg !97
  call fastcc void @_ZN1CC2Ev(%class.C* %0) #4, !dbg !98
  ret void, !dbg !98
}
; CHECK: define void @_ZN1CC1Ev({0: {0: {0: si32 (...)**}}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN1CC2Ev(%1)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1CC2Ev(%class.C*) unnamed_addr #0 align 2 !dbg !128 {
  call void @llvm.dbg.value(metadata %class.C* %0, metadata !129, metadata !DIExpression()), !dbg !130
  %2 = getelementptr inbounds %class.C, %class.C* %0, i64 0, i32 0, !dbg !131
  call fastcc void @_ZN1BC2Ev(%class.B* %2) #4, !dbg !131
  %3 = getelementptr inbounds %class.C, %class.C* %0, i64 0, i32 0, i32 0, i32 0, !dbg !131
  %4 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1C, i64 0, i32 0, i64 2, !dbg !131
  %5 = bitcast i8** %4 to i32 (...)**, !dbg !131
  store i32 (...)** %5, i32 (...)*** %3, align 8, !dbg !131
  ret void, !dbg !131
}
; CHECK: define void @_ZN1CC2Ev({0: {0: {0: si32 (...)**}}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: si32 (...)**}}* %2 = ptrshift %1, 8 * 0, 1 * 0
; CHECK:   call @_ZN1BC2Ev(%2)
; CHECK:   si32 (...)*** %3 = ptrshift %1, 8 * 0, 1 * 0, 1 * 0, 1 * 0
; CHECK:   si8** %4 = ptrshift @_ZTV1C, 32 * 0, 1 * 0, 8 * 2
; CHECK:   si32 (...)** %5 = bitcast %4
; CHECK:   store %3, %5, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

declare i32 @__gxx_personality_v0(...)
; CHECK: declare si32 @__gxx_personality_v0(...)

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main() local_unnamed_addr #2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !dbg !51 {
  %1 = alloca %class.B, align 8
  %2 = alloca %class.C, align 8
  call void @llvm.dbg.value(metadata %class.B* %1, metadata !52, metadata !DIExpression(DW_OP_deref)), !dbg !63
  call fastcc void @_ZN1BC1Ev(%class.B* nonnull %1) #4, !dbg !64
  call void @llvm.dbg.value(metadata %class.C* %2, metadata !65, metadata !DIExpression(DW_OP_deref)), !dbg !63
  call fastcc void @_ZN1CC1Ev(%class.C* nonnull %2) #4, !dbg !76
  %3 = getelementptr inbounds %class.B, %class.B* %1, i64 0, i32 0, !dbg !77
  call fastcc void @_Z3runP1A(%class.A* nonnull %3), !dbg !79
  %4 = getelementptr inbounds %class.C, %class.C* %2, i64 0, i32 0, i32 0, !dbg !80
  call fastcc void @_Z3runP1A(%class.A* nonnull %4), !dbg !81
  ret i32 0, !dbg !82
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: si32 (...)**}}* $1 = allocate {0: {0: si32 (...)**}}, 1, align 8
; CHECK:   {0: {0: {0: si32 (...)**}}}* $2 = allocate {0: {0: {0: si32 (...)**}}}, 1, align 8
; CHECK:   call @_ZN1BC1Ev($1)
; CHECK:   call @_ZN1CC1Ev($2)
; CHECK:   {0: si32 (...)**}* %3 = ptrshift $1, 8 * 0, 1 * 0
; CHECK:   call @_Z3runP1A(%3)
; CHECK:   {0: si32 (...)**}* %4 = ptrshift $2, 8 * 0, 1 * 0, 1 * 0
; CHECK:   call @_Z3runP1A(%4)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #3

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone speculatable }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!7, !8, !9, !10}
!llvm.ident = !{!11}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3, nameTableKind: GNU)
!1 = !DIFile(filename: "try-catch.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4}
!4 = !DIGlobalVariableExpression(var: !5, expr: !DIExpression())
!5 = distinct !DIGlobalVariable(name: "G", scope: !0, file: !1, line: 1, type: !6, isLocal: false, isDefinition: true)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{i32 2, !"Dwarf Version", i32 4}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!9 = !{i32 1, !"wchar_size", i32 4}
!10 = !{i32 7, !"PIC Level", i32 2}
!11 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!12 = distinct !DISubprogram(name: "h", linkageName: "_Z1hi", scope: !1, file: !1, line: 21, type: !13, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!13 = !DISubroutineType(types: !14)
!14 = !{null, !6}
!15 = !DILocalVariable(name: "x", arg: 1, scope: !12, file: !1, line: 21, type: !6)
!16 = !DILocation(line: 0, scope: !12)
!17 = !DILocation(line: 21, column: 16, scope: !12)
!18 = distinct !DISubprogram(name: "hh", linkageName: "_Z2hhi", scope: !1, file: !1, line: 23, type: !19, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!19 = !DISubroutineType(types: !20)
!20 = !{!6, !6}
!21 = !DILocalVariable(name: "x", arg: 1, scope: !18, file: !1, line: 23, type: !6)
!22 = !DILocation(line: 0, scope: !18)
!23 = !DILocation(line: 24, column: 12, scope: !18)
!24 = !DILocation(line: 24, column: 3, scope: !18)
!25 = distinct !DISubprogram(name: "run", linkageName: "_Z3runP1A", scope: !1, file: !1, line: 27, type: !26, scopeLine: 27, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!26 = !DISubroutineType(types: !27)
!27 = !{null, !28}
!28 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64)
!29 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "A", file: !1, line: 3, size: 64, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !30, vtableHolder: !29, identifier: "_ZTS1A")
!30 = !{!31, !36, !40}
!31 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$A", scope: !1, file: !1, baseType: !32, size: 64, flags: DIFlagArtificial)
!32 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !33, size: 64)
!33 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "__vtbl_ptr_type", baseType: !34, size: 64)
!34 = !DISubroutineType(types: !35)
!35 = !{!6}
!36 = !DISubprogram(name: "f", linkageName: "_ZN1A1fEi", scope: !29, file: !1, line: 5, type: !37, scopeLine: 5, containingType: !29, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!37 = !DISubroutineType(types: !38)
!38 = !{null, !39, !6}
!39 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!40 = !DISubprogram(name: "g", linkageName: "_ZN1A1gEv", scope: !29, file: !1, line: 6, type: !41, scopeLine: 6, containingType: !29, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!41 = !DISubroutineType(types: !42)
!42 = !{!6, !39}
!43 = !DILocalVariable(name: "p", arg: 1, scope: !25, file: !1, line: 27, type: !28)
!44 = !DILocation(line: 0, scope: !25)
!45 = !DILocation(line: 29, column: 6, scope: !25)
!46 = !DILocation(line: 30, column: 10, scope: !25)
!47 = !DILocalVariable(name: "x", scope: !25, file: !1, line: 28, type: !6)
!48 = !DILocation(line: 31, column: 3, scope: !25)
!49 = !DILocation(line: 32, column: 7, scope: !25)
!50 = !DILocation(line: 33, column: 1, scope: !25)
!51 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 35, type: !34, scopeLine: 35, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!52 = !DILocalVariable(name: "b", scope: !51, file: !1, line: 36, type: !53)
!53 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "B", file: !1, line: 9, size: 64, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !54, vtableHolder: !29, identifier: "_ZTS1B")
!54 = !{!55, !56, !60}
!55 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !53, baseType: !29, flags: DIFlagPublic, extraData: i32 0)
!56 = !DISubprogram(name: "f", linkageName: "_ZN1B1fEi", scope: !53, file: !1, line: 11, type: !57, scopeLine: 11, containingType: !53, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!57 = !DISubroutineType(types: !58)
!58 = !{null, !59, !6}
!59 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !53, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!60 = !DISubprogram(name: "g", linkageName: "_ZN1B1gEv", scope: !53, file: !1, line: 12, type: !61, scopeLine: 12, containingType: !53, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!61 = !DISubroutineType(types: !62)
!62 = !{!6, !59}
!63 = !DILocation(line: 0, scope: !51)
!64 = !DILocation(line: 36, column: 5, scope: !51)
!65 = !DILocalVariable(name: "c", scope: !51, file: !1, line: 37, type: !66)
!66 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "C", file: !1, line: 15, size: 64, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !67, vtableHolder: !29, identifier: "_ZTS1C")
!67 = !{!68, !69, !73}
!68 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !66, baseType: !53, flags: DIFlagPublic, extraData: i32 0)
!69 = !DISubprogram(name: "f", linkageName: "_ZN1C1fEi", scope: !66, file: !1, line: 17, type: !70, scopeLine: 17, containingType: !66, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!70 = !DISubroutineType(types: !71)
!71 = !{null, !72, !6}
!72 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !66, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!73 = !DISubprogram(name: "g", linkageName: "_ZN1C1gEv", scope: !66, file: !1, line: 18, type: !74, scopeLine: 18, containingType: !66, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!74 = !DISubroutineType(types: !75)
!75 = !{!6, !72}
!76 = !DILocation(line: 37, column: 5, scope: !51)
!77 = !DILocation(line: 39, column: 9, scope: !78)
!78 = distinct !DILexicalBlock(scope: !51, file: !1, line: 38, column: 7)
!79 = !DILocation(line: 39, column: 5, scope: !78)
!80 = !DILocation(line: 40, column: 9, scope: !78)
!81 = !DILocation(line: 40, column: 5, scope: !78)
!82 = !DILocation(line: 43, column: 3, scope: !51)
!83 = distinct !DISubprogram(name: "B", linkageName: "_ZN1BC1Ev", scope: !53, file: !1, line: 9, type: !84, scopeLine: 9, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !86, retainedNodes: !2)
!84 = !DISubroutineType(types: !85)
!85 = !{null, !59}
!86 = !DISubprogram(name: "B", scope: !53, type: !84, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!87 = !DILocalVariable(name: "this", arg: 1, scope: !83, type: !88, flags: DIFlagArtificial | DIFlagObjectPointer)
!88 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !53, size: 64)
!89 = !DILocation(line: 0, scope: !83)
!90 = !DILocation(line: 9, column: 7, scope: !83)
!91 = distinct !DISubprogram(name: "C", linkageName: "_ZN1CC1Ev", scope: !66, file: !1, line: 15, type: !92, scopeLine: 15, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !94, retainedNodes: !2)
!92 = !DISubroutineType(types: !93)
!93 = !{null, !72}
!94 = !DISubprogram(name: "C", scope: !66, type: !92, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!95 = !DILocalVariable(name: "this", arg: 1, scope: !91, type: !96, flags: DIFlagArtificial | DIFlagObjectPointer)
!96 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !66, size: 64)
!97 = !DILocation(line: 0, scope: !91)
!98 = !DILocation(line: 15, column: 7, scope: !91)
!99 = distinct !DISubprogram(name: "B", linkageName: "_ZN1BC2Ev", scope: !53, file: !1, line: 9, type: !84, scopeLine: 9, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !86, retainedNodes: !2)
!100 = !DILocalVariable(name: "this", arg: 1, scope: !99, type: !88, flags: DIFlagArtificial | DIFlagObjectPointer)
!101 = !DILocation(line: 0, scope: !99)
!102 = !DILocation(line: 9, column: 7, scope: !99)
!103 = distinct !DISubprogram(name: "A", linkageName: "_ZN1AC2Ev", scope: !29, file: !1, line: 3, type: !104, scopeLine: 3, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !106, retainedNodes: !2)
!104 = !DISubroutineType(types: !105)
!105 = !{null, !39}
!106 = !DISubprogram(name: "A", scope: !29, type: !104, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!107 = !DILocalVariable(name: "this", arg: 1, scope: !103, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!108 = !DILocation(line: 0, scope: !103)
!109 = !DILocation(line: 3, column: 7, scope: !103)
!110 = distinct !DISubprogram(name: "f", linkageName: "_ZN1B1fEi", scope: !53, file: !1, line: 11, type: !57, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !56, retainedNodes: !2)
!111 = !DILocalVariable(name: "this", arg: 1, scope: !110, type: !88, flags: DIFlagArtificial | DIFlagObjectPointer)
!112 = !DILocation(line: 0, scope: !110)
!113 = !DILocalVariable(name: "x", arg: 2, scope: !110, file: !1, line: 11, type: !6)
!114 = !DILocation(line: 11, column: 34, scope: !110)
!115 = distinct !DISubprogram(name: "g", linkageName: "_ZN1B1gEv", scope: !53, file: !1, line: 12, type: !61, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !60, retainedNodes: !2)
!116 = !DILocalVariable(name: "this", arg: 1, scope: !115, type: !88, flags: DIFlagArtificial | DIFlagObjectPointer)
!117 = !DILocation(line: 0, scope: !115)
!118 = !DILocation(line: 12, column: 21, scope: !115)
!119 = distinct !DISubprogram(name: "f", linkageName: "_ZN1A1fEi", scope: !29, file: !1, line: 5, type: !37, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !36, retainedNodes: !2)
!120 = !DILocalVariable(name: "this", arg: 1, scope: !119, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!121 = !DILocation(line: 0, scope: !119)
!122 = !DILocalVariable(name: "x", arg: 2, scope: !119, file: !1, line: 5, type: !6)
!123 = !DILocation(line: 5, column: 26, scope: !119)
!124 = distinct !DISubprogram(name: "g", linkageName: "_ZN1A1gEv", scope: !29, file: !1, line: 6, type: !41, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !40, retainedNodes: !2)
!125 = !DILocalVariable(name: "this", arg: 1, scope: !124, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!126 = !DILocation(line: 0, scope: !124)
!127 = !DILocation(line: 6, column: 21, scope: !124)
!128 = distinct !DISubprogram(name: "C", linkageName: "_ZN1CC2Ev", scope: !66, file: !1, line: 15, type: !92, scopeLine: 15, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !94, retainedNodes: !2)
!129 = !DILocalVariable(name: "this", arg: 1, scope: !128, type: !96, flags: DIFlagArtificial | DIFlagObjectPointer)
!130 = !DILocation(line: 0, scope: !128)
!131 = !DILocation(line: 15, column: 7, scope: !128)
!132 = distinct !DISubprogram(name: "f", linkageName: "_ZN1C1fEi", scope: !66, file: !1, line: 17, type: !70, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !69, retainedNodes: !2)
!133 = !DILocalVariable(name: "this", arg: 1, scope: !132, type: !96, flags: DIFlagArtificial | DIFlagObjectPointer)
!134 = !DILocation(line: 0, scope: !132)
!135 = !DILocalVariable(name: "x", arg: 2, scope: !132, file: !1, line: 17, type: !6)
!136 = !DILocation(line: 17, column: 35, scope: !132)
!137 = distinct !DISubprogram(name: "g", linkageName: "_ZN1C1gEv", scope: !66, file: !1, line: 18, type: !74, scopeLine: 18, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !73, retainedNodes: !2)
!138 = !DILocalVariable(name: "this", arg: 1, scope: !137, type: !96, flags: DIFlagArtificial | DIFlagObjectPointer)
!139 = !DILocation(line: 0, scope: !137)
!140 = !DILocation(line: 18, column: 21, scope: !137)
