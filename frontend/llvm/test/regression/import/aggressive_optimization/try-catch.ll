; ModuleID = 'try-catch.cpp.pp.bc'
source_filename = "try-catch.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

%class.A = type { i32 (...)** }
%class.B = type { %class.A }
%class.C = type { %class.B }

@_ZTI1A = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1A, i32 0, i32 0) }
; CHECK: define {0: si8*, 8: si8*}* @_ZTI1A, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv117__class_type_infoE, 8 * 2
; CHECK:   si8* %2 = ptrshift @_ZTS1A, 3 * 0, 1 * 0
; CHECK:   si8* %3 = bitcast %1
; CHECK:   store @_ZTI1A, {0: %3, 8: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTI1B = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1B, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*) }
; CHECK: define {0: si8*, 8: si8*, 16: si8*}* @_ZTI1B, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv120__si_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1A
; CHECK:   si8* %3 = ptrshift @_ZTS1B, 3 * 0, 1 * 0
; CHECK:   si8* %4 = bitcast %1
; CHECK:   store @_ZTI1B, {0: %4, 8: %3, 16: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTI1C = internal constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1C, i32 0, i32 0), i8* bitcast ({ i8*, i8*, i8* }* @_ZTI1B to i8*) }
; CHECK: define {0: si8*, 8: si8*, 16: si8*}* @_ZTI1C, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv120__si_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1B
; CHECK:   si8* %3 = ptrshift @_ZTS1C, 3 * 0, 1 * 0
; CHECK:   si8* %4 = bitcast %1
; CHECK:   store @_ZTI1C, {0: %4, 8: %3, 16: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTS1A = internal constant [3 x i8] c"1A\00"
; CHECK: define [3 x si8]* @_ZTS1A, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1A, [49, 65, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1B = internal constant [3 x i8] c"1B\00"
; CHECK: define [3 x si8]* @_ZTS1B, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1B, [49, 66, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1C = internal constant [3 x i8] c"1C\00"
; CHECK: define [3 x si8]* @_ZTS1C, init {
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
define internal fastcc void @_Z1hi(i32) unnamed_addr #0 !dbg !11 {
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !14, metadata !15), !dbg !16
  ret void, !dbg !17
}
; CHECK: define void @_Z1hi(si32 %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc i32 @_Z2hhi(i32) unnamed_addr #0 !dbg !18 {
  call void @llvm.dbg.value(metadata i32 %0, i64 0, metadata !21, metadata !15), !dbg !22
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
  call void @llvm.dbg.value(metadata %class.A* %0, i64 0, metadata !43, metadata !15), !dbg !44
  %2 = bitcast %class.A* %0 to void (%class.A*, i32)***, !dbg !45
  %3 = load void (%class.A*, i32)**, void (%class.A*, i32)*** %2, align 8, !dbg !45
  %4 = load void (%class.A*, i32)*, void (%class.A*, i32)** %3, align 8, !dbg !45
  call void %4(%class.A* %0, i32 12), !dbg !45
  %5 = bitcast %class.A* %0 to i32 (%class.A*)***, !dbg !46
  %6 = load i32 (%class.A*)**, i32 (%class.A*)*** %5, align 8, !dbg !46
  %7 = getelementptr inbounds i32 (%class.A*)*, i32 (%class.A*)** %6, i64 1, !dbg !46
  %8 = load i32 (%class.A*)*, i32 (%class.A*)** %7, align 8, !dbg !46
  %9 = call i32 %8(%class.A* %0), !dbg !46
  call void @llvm.dbg.value(metadata i32 %9, i64 0, metadata !47, metadata !15), !dbg !48
  call fastcc void @_Z1hi(i32 14), !dbg !49
  %10 = call fastcc i32 @_Z2hhi(i32 15), !dbg !50
  call void @llvm.dbg.value(metadata i32 %10, i64 0, metadata !47, metadata !15), !dbg !48
  ret void, !dbg !51
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
define internal void @_ZN1A1fEi(%class.A*, i32) unnamed_addr #0 align 2 !dbg !121 {
  call void @llvm.dbg.value(metadata %class.A* %0, i64 0, metadata !122, metadata !15), !dbg !123
  call void @llvm.dbg.value(metadata i32 %1, i64 0, metadata !124, metadata !15), !dbg !125
  ret void, !dbg !126
}
; CHECK: define void @_ZN1A1fEi({0: si32 (...)**}* %1, si32 %2) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal i32 @_ZN1A1gEv(%class.A*) unnamed_addr #0 align 2 !dbg !127 {
  call void @llvm.dbg.value(metadata %class.A* %0, i64 0, metadata !128, metadata !15), !dbg !129
  ret i32 0, !dbg !130
}
; CHECK: define si32 @_ZN1A1gEv({0: si32 (...)**}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1AC2Ev(%class.A*) unnamed_addr #0 align 2 !dbg !104 {
  call void @llvm.dbg.value(metadata %class.A* %0, i64 0, metadata !108, metadata !15), !dbg !109
  %2 = getelementptr inbounds %class.A, %class.A* %0, i64 0, i32 0, !dbg !110
  %3 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1A, i64 0, i32 0, i64 2, !dbg !110
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !110
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !110
  ret void, !dbg !110
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
define internal void @_ZN1B1fEi(%class.B*, i32) unnamed_addr #0 align 2 !dbg !111 {
  call void @llvm.dbg.value(metadata %class.B* %0, i64 0, metadata !112, metadata !15), !dbg !113
  call void @llvm.dbg.value(metadata i32 %1, i64 0, metadata !114, metadata !15), !dbg !115
  ret void, !dbg !116
}
; CHECK: define void @_ZN1B1fEi({0: {0: si32 (...)**}}* %1, si32 %2) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal i32 @_ZN1B1gEv(%class.B*) unnamed_addr #0 align 2 !dbg !117 {
  call void @llvm.dbg.value(metadata %class.B* %0, i64 0, metadata !118, metadata !15), !dbg !119
  ret i32 0, !dbg !120
}
; CHECK: define si32 @_ZN1B1gEv({0: {0: si32 (...)**}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1BC1Ev(%class.B*) unnamed_addr #0 align 2 !dbg !84 {
  call void @llvm.dbg.value(metadata %class.B* %0, i64 0, metadata !88, metadata !15), !dbg !90
  call fastcc void @_ZN1BC2Ev(%class.B* %0) #4, !dbg !91
  ret void, !dbg !91
}
; CHECK: define void @_ZN1BC1Ev({0: {0: si32 (...)**}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN1BC2Ev(%1)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1BC2Ev(%class.B*) unnamed_addr #0 align 2 !dbg !100 {
  call void @llvm.dbg.value(metadata %class.B* %0, i64 0, metadata !101, metadata !15), !dbg !102
  %2 = getelementptr inbounds %class.B, %class.B* %0, i64 0, i32 0, !dbg !103
  call fastcc void @_ZN1AC2Ev(%class.A* %2) #4, !dbg !103
  %3 = getelementptr inbounds %class.B, %class.B* %0, i64 0, i32 0, i32 0, !dbg !103
  %4 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1B, i64 0, i32 0, i64 2, !dbg !103
  %5 = bitcast i8** %4 to i32 (...)**, !dbg !103
  store i32 (...)** %5, i32 (...)*** %3, align 8, !dbg !103
  ret void, !dbg !103
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
define internal void @_ZN1C1fEi(%class.C*, i32) unnamed_addr #0 align 2 !dbg !135 {
  call void @llvm.dbg.value(metadata %class.C* %0, i64 0, metadata !136, metadata !15), !dbg !137
  call void @llvm.dbg.value(metadata i32 %1, i64 0, metadata !138, metadata !15), !dbg !139
  ret void, !dbg !140
}
; CHECK: define void @_ZN1C1fEi({0: {0: {0: si32 (...)**}}}* %1, si32 %2) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal i32 @_ZN1C1gEv(%class.C*) unnamed_addr #0 align 2 !dbg !141 {
  call void @llvm.dbg.value(metadata %class.C* %0, i64 0, metadata !142, metadata !15), !dbg !143
  ret i32 1, !dbg !144
}
; CHECK: define si32 @_ZN1C1gEv({0: {0: {0: si32 (...)**}}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1CC1Ev(%class.C*) unnamed_addr #0 align 2 !dbg !92 {
  call void @llvm.dbg.value(metadata %class.C* %0, i64 0, metadata !96, metadata !15), !dbg !98
  call fastcc void @_ZN1CC2Ev(%class.C* %0) #4, !dbg !99
  ret void, !dbg !99
}
; CHECK: define void @_ZN1CC1Ev({0: {0: {0: si32 (...)**}}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   call @_ZN1CC2Ev(%1)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1CC2Ev(%class.C*) unnamed_addr #0 align 2 !dbg !131 {
  call void @llvm.dbg.value(metadata %class.C* %0, i64 0, metadata !132, metadata !15), !dbg !133
  %2 = getelementptr inbounds %class.C, %class.C* %0, i64 0, i32 0, !dbg !134
  call fastcc void @_ZN1BC2Ev(%class.B* %2) #4, !dbg !134
  %3 = getelementptr inbounds %class.C, %class.C* %0, i64 0, i32 0, i32 0, i32 0, !dbg !134
  %4 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1C, i64 0, i32 0, i64 2, !dbg !134
  %5 = bitcast i8** %4 to i32 (...)**, !dbg !134
  store i32 (...)** %5, i32 (...)*** %3, align 8, !dbg !134
  ret void, !dbg !134
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
define i32 @main() local_unnamed_addr #2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !dbg !52 {
  %1 = alloca %class.B, align 8
  %2 = alloca %class.C, align 8
  call void @llvm.dbg.value(metadata %class.B* %1, i64 0, metadata !53, metadata !64), !dbg !65
  call fastcc void @_ZN1BC1Ev(%class.B* nonnull %1) #4, !dbg !65
  call void @llvm.dbg.value(metadata %class.C* %2, i64 0, metadata !66, metadata !64), !dbg !77
  call fastcc void @_ZN1CC1Ev(%class.C* nonnull %2) #4, !dbg !77
  %3 = getelementptr inbounds %class.B, %class.B* %1, i64 0, i32 0, !dbg !78
  call fastcc void @_Z3runP1A(%class.A* %3), !dbg !80
  %4 = getelementptr inbounds %class.C, %class.C* %2, i64 0, i32 0, i32 0, !dbg !81
  call fastcc void @_Z3runP1A(%class.A* %4), !dbg !82
  ret i32 0, !dbg !83
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #3

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!7, !8, !9}
!llvm.ident = !{!10}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, globals: !3)
!1 = !DIFile(filename: "try-catch.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{!4}
!4 = !DIGlobalVariableExpression(var: !5)
!5 = distinct !DIGlobalVariable(name: "G", scope: !0, file: !1, line: 1, type: !6, isLocal: false, isDefinition: true)
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{i32 2, !"Dwarf Version", i32 4}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!9 = !{i32 1, !"PIC Level", i32 2}
!10 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!11 = distinct !DISubprogram(name: "h", linkageName: "_Z1hi", scope: !1, file: !1, line: 21, type: !12, isLocal: false, isDefinition: true, scopeLine: 21, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!12 = !DISubroutineType(types: !13)
!13 = !{null, !6}
!14 = !DILocalVariable(name: "x", arg: 1, scope: !11, file: !1, line: 21, type: !6)
!15 = !DIExpression()
!16 = !DILocation(line: 21, column: 12, scope: !11)
!17 = !DILocation(line: 21, column: 16, scope: !11)
!18 = distinct !DISubprogram(name: "hh", linkageName: "_Z2hhi", scope: !1, file: !1, line: 23, type: !19, isLocal: false, isDefinition: true, scopeLine: 23, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!19 = !DISubroutineType(types: !20)
!20 = !{!6, !6}
!21 = !DILocalVariable(name: "x", arg: 1, scope: !18, file: !1, line: 23, type: !6)
!22 = !DILocation(line: 23, column: 12, scope: !18)
!23 = !DILocation(line: 24, column: 12, scope: !18)
!24 = !DILocation(line: 24, column: 3, scope: !18)
!25 = distinct !DISubprogram(name: "run", linkageName: "_Z3runP1A", scope: !1, file: !1, line: 27, type: !26, isLocal: false, isDefinition: true, scopeLine: 27, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!26 = !DISubroutineType(types: !27)
!27 = !{null, !28}
!28 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64)
!29 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "A", file: !1, line: 3, size: 64, elements: !30, vtableHolder: !29, identifier: "_ZTS1A")
!30 = !{!31, !36, !40}
!31 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$A", scope: !1, file: !1, baseType: !32, size: 64, flags: DIFlagArtificial)
!32 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !33, size: 64)
!33 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "__vtbl_ptr_type", baseType: !34, size: 64)
!34 = !DISubroutineType(types: !35)
!35 = !{!6}
!36 = !DISubprogram(name: "f", linkageName: "_ZN1A1fEi", scope: !29, file: !1, line: 5, type: !37, isLocal: false, isDefinition: false, scopeLine: 5, containingType: !29, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!37 = !DISubroutineType(types: !38)
!38 = !{null, !39, !6}
!39 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !29, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!40 = !DISubprogram(name: "g", linkageName: "_ZN1A1gEv", scope: !29, file: !1, line: 6, type: !41, isLocal: false, isDefinition: false, scopeLine: 6, containingType: !29, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!41 = !DISubroutineType(types: !42)
!42 = !{!6, !39}
!43 = !DILocalVariable(name: "p", arg: 1, scope: !25, file: !1, line: 27, type: !28)
!44 = !DILocation(line: 27, column: 13, scope: !25)
!45 = !DILocation(line: 29, column: 6, scope: !25)
!46 = !DILocation(line: 30, column: 10, scope: !25)
!47 = !DILocalVariable(name: "x", scope: !25, file: !1, line: 28, type: !6)
!48 = !DILocation(line: 28, column: 7, scope: !25)
!49 = !DILocation(line: 31, column: 3, scope: !25)
!50 = !DILocation(line: 32, column: 7, scope: !25)
!51 = !DILocation(line: 33, column: 1, scope: !25)
!52 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 35, type: !34, isLocal: false, isDefinition: true, scopeLine: 35, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!53 = !DILocalVariable(name: "b", scope: !52, file: !1, line: 36, type: !54)
!54 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "B", file: !1, line: 9, size: 64, elements: !55, vtableHolder: !29, identifier: "_ZTS1B")
!55 = !{!56, !57, !61}
!56 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !54, baseType: !29, flags: DIFlagPublic)
!57 = !DISubprogram(name: "f", linkageName: "_ZN1B1fEi", scope: !54, file: !1, line: 11, type: !58, isLocal: false, isDefinition: false, scopeLine: 11, containingType: !54, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!58 = !DISubroutineType(types: !59)
!59 = !{null, !60, !6}
!60 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !54, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!61 = !DISubprogram(name: "g", linkageName: "_ZN1B1gEv", scope: !54, file: !1, line: 12, type: !62, isLocal: false, isDefinition: false, scopeLine: 12, containingType: !54, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!62 = !DISubroutineType(types: !63)
!63 = !{!6, !60}
!64 = !DIExpression(DW_OP_deref)
!65 = !DILocation(line: 36, column: 5, scope: !52)
!66 = !DILocalVariable(name: "c", scope: !52, file: !1, line: 37, type: !67)
!67 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "C", file: !1, line: 15, size: 64, elements: !68, vtableHolder: !29, identifier: "_ZTS1C")
!68 = !{!69, !70, !74}
!69 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !67, baseType: !54, flags: DIFlagPublic)
!70 = !DISubprogram(name: "f", linkageName: "_ZN1C1fEi", scope: !67, file: !1, line: 17, type: !71, isLocal: false, isDefinition: false, scopeLine: 17, containingType: !67, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!71 = !DISubroutineType(types: !72)
!72 = !{null, !73, !6}
!73 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !67, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!74 = !DISubprogram(name: "g", linkageName: "_ZN1C1gEv", scope: !67, file: !1, line: 18, type: !75, isLocal: false, isDefinition: false, scopeLine: 18, containingType: !67, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, isOptimized: false)
!75 = !DISubroutineType(types: !76)
!76 = !{!6, !73}
!77 = !DILocation(line: 37, column: 5, scope: !52)
!78 = !DILocation(line: 39, column: 9, scope: !79)
!79 = distinct !DILexicalBlock(scope: !52, file: !1, line: 38, column: 7)
!80 = !DILocation(line: 39, column: 5, scope: !79)
!81 = !DILocation(line: 40, column: 9, scope: !79)
!82 = !DILocation(line: 40, column: 5, scope: !79)
!83 = !DILocation(line: 43, column: 3, scope: !52)
!84 = distinct !DISubprogram(name: "B", linkageName: "_ZN1BC1Ev", scope: !54, file: !1, line: 9, type: !85, isLocal: false, isDefinition: true, scopeLine: 9, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !87, variables: !2)
!85 = !DISubroutineType(types: !86)
!86 = !{null, !60}
!87 = !DISubprogram(name: "B", scope: !54, type: !85, isLocal: false, isDefinition: false, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!88 = !DILocalVariable(name: "this", arg: 1, scope: !84, type: !89, flags: DIFlagArtificial | DIFlagObjectPointer)
!89 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !54, size: 64)
!90 = !DILocation(line: 0, scope: !84)
!91 = !DILocation(line: 9, column: 7, scope: !84)
!92 = distinct !DISubprogram(name: "C", linkageName: "_ZN1CC1Ev", scope: !67, file: !1, line: 15, type: !93, isLocal: false, isDefinition: true, scopeLine: 15, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !95, variables: !2)
!93 = !DISubroutineType(types: !94)
!94 = !{null, !73}
!95 = !DISubprogram(name: "C", scope: !67, type: !93, isLocal: false, isDefinition: false, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!96 = !DILocalVariable(name: "this", arg: 1, scope: !92, type: !97, flags: DIFlagArtificial | DIFlagObjectPointer)
!97 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !67, size: 64)
!98 = !DILocation(line: 0, scope: !92)
!99 = !DILocation(line: 15, column: 7, scope: !92)
!100 = distinct !DISubprogram(name: "B", linkageName: "_ZN1BC2Ev", scope: !54, file: !1, line: 9, type: !85, isLocal: false, isDefinition: true, scopeLine: 9, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !87, variables: !2)
!101 = !DILocalVariable(name: "this", arg: 1, scope: !100, type: !89, flags: DIFlagArtificial | DIFlagObjectPointer)
!102 = !DILocation(line: 0, scope: !100)
!103 = !DILocation(line: 9, column: 7, scope: !100)
!104 = distinct !DISubprogram(name: "A", linkageName: "_ZN1AC2Ev", scope: !29, file: !1, line: 3, type: !105, isLocal: false, isDefinition: true, scopeLine: 3, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !107, variables: !2)
!105 = !DISubroutineType(types: !106)
!106 = !{null, !39}
!107 = !DISubprogram(name: "A", scope: !29, type: !105, isLocal: false, isDefinition: false, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!108 = !DILocalVariable(name: "this", arg: 1, scope: !104, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!109 = !DILocation(line: 0, scope: !104)
!110 = !DILocation(line: 3, column: 7, scope: !104)
!111 = distinct !DISubprogram(name: "f", linkageName: "_ZN1B1fEi", scope: !54, file: !1, line: 11, type: !58, isLocal: false, isDefinition: true, scopeLine: 11, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !57, variables: !2)
!112 = !DILocalVariable(name: "this", arg: 1, scope: !111, type: !89, flags: DIFlagArtificial | DIFlagObjectPointer)
!113 = !DILocation(line: 0, scope: !111)
!114 = !DILocalVariable(name: "x", arg: 2, scope: !111, file: !1, line: 11, type: !6)
!115 = !DILocation(line: 11, column: 22, scope: !111)
!116 = !DILocation(line: 11, column: 34, scope: !111)
!117 = distinct !DISubprogram(name: "g", linkageName: "_ZN1B1gEv", scope: !54, file: !1, line: 12, type: !62, isLocal: false, isDefinition: true, scopeLine: 12, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !61, variables: !2)
!118 = !DILocalVariable(name: "this", arg: 1, scope: !117, type: !89, flags: DIFlagArtificial | DIFlagObjectPointer)
!119 = !DILocation(line: 0, scope: !117)
!120 = !DILocation(line: 12, column: 21, scope: !117)
!121 = distinct !DISubprogram(name: "f", linkageName: "_ZN1A1fEi", scope: !29, file: !1, line: 5, type: !37, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !36, variables: !2)
!122 = !DILocalVariable(name: "this", arg: 1, scope: !121, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!123 = !DILocation(line: 0, scope: !121)
!124 = !DILocalVariable(name: "x", arg: 2, scope: !121, file: !1, line: 5, type: !6)
!125 = !DILocation(line: 5, column: 22, scope: !121)
!126 = !DILocation(line: 5, column: 26, scope: !121)
!127 = distinct !DISubprogram(name: "g", linkageName: "_ZN1A1gEv", scope: !29, file: !1, line: 6, type: !41, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !40, variables: !2)
!128 = !DILocalVariable(name: "this", arg: 1, scope: !127, type: !28, flags: DIFlagArtificial | DIFlagObjectPointer)
!129 = !DILocation(line: 0, scope: !127)
!130 = !DILocation(line: 6, column: 21, scope: !127)
!131 = distinct !DISubprogram(name: "C", linkageName: "_ZN1CC2Ev", scope: !67, file: !1, line: 15, type: !93, isLocal: false, isDefinition: true, scopeLine: 15, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !95, variables: !2)
!132 = !DILocalVariable(name: "this", arg: 1, scope: !131, type: !97, flags: DIFlagArtificial | DIFlagObjectPointer)
!133 = !DILocation(line: 0, scope: !131)
!134 = !DILocation(line: 15, column: 7, scope: !131)
!135 = distinct !DISubprogram(name: "f", linkageName: "_ZN1C1fEi", scope: !67, file: !1, line: 17, type: !71, isLocal: false, isDefinition: true, scopeLine: 17, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !70, variables: !2)
!136 = !DILocalVariable(name: "this", arg: 1, scope: !135, type: !97, flags: DIFlagArtificial | DIFlagObjectPointer)
!137 = !DILocation(line: 0, scope: !135)
!138 = !DILocalVariable(name: "x", arg: 2, scope: !135, file: !1, line: 17, type: !6)
!139 = !DILocation(line: 17, column: 22, scope: !135)
!140 = !DILocation(line: 17, column: 35, scope: !135)
!141 = distinct !DISubprogram(name: "g", linkageName: "_ZN1C1gEv", scope: !67, file: !1, line: 18, type: !75, isLocal: false, isDefinition: true, scopeLine: 18, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !74, variables: !2)
!142 = !DILocalVariable(name: "this", arg: 1, scope: !141, type: !97, flags: DIFlagArtificial | DIFlagObjectPointer)
!143 = !DILocation(line: 0, scope: !141)
!144 = !DILocation(line: 18, column: 21, scope: !141)
