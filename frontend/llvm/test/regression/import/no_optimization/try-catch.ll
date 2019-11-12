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

@G = global i32 0, align 4, !dbg !0
; CHECK: define si32* @G, align 4, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @G, 0, align 1
; CHECK: }
; CHECK: }

@_ZTI1A = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1A, i32 0, i32 0) }, align 8
; CHECK: define {0: si8*, 8: si8*}* @_ZTI1A, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv117__class_type_infoE, 8 * 2
; CHECK:   si8* %2 = ptrshift @_ZTS1A, 3 * 0, 1 * 0
; CHECK:   si8* %3 = bitcast %1
; CHECK:   store @_ZTI1A, {0: %3, 8: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTI1B = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1B, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*) }, align 8
; CHECK: define {0: si8*, 8: si8*, 16: si8*}* @_ZTI1B, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv120__si_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1A
; CHECK:   si8* %3 = ptrshift @_ZTS1B, 3 * 0, 1 * 0
; CHECK:   si8* %4 = bitcast %1
; CHECK:   store @_ZTI1B, {0: %4, 8: %3, 16: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTI1C = linkonce_odr constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1C, i32 0, i32 0), i8* bitcast ({ i8*, i8*, i8* }* @_ZTI1B to i8*) }, align 8
; CHECK: define {0: si8*, 8: si8*, 16: si8*}* @_ZTI1C, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv120__si_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1B
; CHECK:   si8* %3 = ptrshift @_ZTS1C, 3 * 0, 1 * 0
; CHECK:   si8* %4 = bitcast %1
; CHECK:   store @_ZTI1C, {0: %4, 8: %3, 16: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTS1A = linkonce_odr constant [3 x i8] c"1A\00", align 1
; CHECK: define [3 x si8]* @_ZTS1A, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1A, [49, 65, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1B = linkonce_odr constant [3 x i8] c"1B\00", align 1
; CHECK: define [3 x si8]* @_ZTS1B, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1B, [49, 66, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1C = linkonce_odr constant [3 x i8] c"1C\00", align 1
; CHECK: define [3 x si8]* @_ZTS1C, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1C, [49, 67, 0], align 1
; CHECK: }
; CHECK: }

@_ZTV1A = linkonce_odr unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i8* bitcast (void (%class.A*, i32)* @_ZN1A1fEi to i8*), i8* bitcast (i32 (%class.A*)* @_ZN1A1gEv to i8*)] }, align 8
; CHECK: define {0: [4 x si8*]}* @_ZTV1A, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZN1A1gEv
; CHECK:   si8* %2 = bitcast @_ZN1A1fEi
; CHECK:   si8* %3 = bitcast @_ZTI1A
; CHECK:   store @_ZTV1A, {0: [null, %3, %2, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTV1B = linkonce_odr unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI1B to i8*), i8* bitcast (void (%class.B*, i32)* @_ZN1B1fEi to i8*), i8* bitcast (i32 (%class.B*)* @_ZN1B1gEv to i8*)] }, align 8
; CHECK: define {0: [4 x si8*]}* @_ZTV1B, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZN1B1gEv
; CHECK:   si8* %2 = bitcast @_ZN1B1fEi
; CHECK:   si8* %3 = bitcast @_ZTI1B
; CHECK:   store @_ZTV1B, {0: [null, %3, %2, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTV1C = linkonce_odr unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI1C to i8*), i8* bitcast (void (%class.C*, i32)* @_ZN1C1fEi to i8*), i8* bitcast (i32 (%class.C*)* @_ZN1C1gEv to i8*)] }, align 8
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
define void @_Z1hi(i32) #0 !dbg !12 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !15, metadata !DIExpression()), !dbg !16
  ret void, !dbg !17
}
; CHECK: define void @_Z1hi(si32 %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $2 = allocate si32, 1, align 4
; CHECK:   store $2, %1, align 4
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define i32 @_Z2hhi(i32) #0 !dbg !18 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  call void @llvm.dbg.declare(metadata i32* %2, metadata !21, metadata !DIExpression()), !dbg !22
  %3 = load i32, i32* %2, align 4, !dbg !23
  %4 = load i32, i32* %2, align 4, !dbg !24
  %5 = mul nsw i32 %3, %4, !dbg !25
  ret i32 %5, !dbg !26
}
; CHECK: define si32 @_Z2hhi(si32 %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32* $2 = allocate si32, 1, align 4
; CHECK:   store $2, %1, align 4
; CHECK:   si32 %3 = load $2, align 4
; CHECK:   si32 %4 = load $2, align 4
; CHECK:   si32 %5 = %3 smul.nw %4
; CHECK:   return %5
; CHECK: }
; CHECK: }

; Function Attrs: noinline ssp uwtable
define void @_Z3runP1A(%class.A*) #2 !dbg !27 {
  %2 = alloca %class.A*, align 8
  %3 = alloca i32, align 4
  store %class.A* %0, %class.A** %2, align 8
  call void @llvm.dbg.declare(metadata %class.A** %2, metadata !45, metadata !DIExpression()), !dbg !46
  call void @llvm.dbg.declare(metadata i32* %3, metadata !47, metadata !DIExpression()), !dbg !48
  %4 = load %class.A*, %class.A** %2, align 8, !dbg !49
  %5 = bitcast %class.A* %4 to void (%class.A*, i32)***, !dbg !50
  %6 = load void (%class.A*, i32)**, void (%class.A*, i32)*** %5, align 8, !dbg !50
  %7 = getelementptr inbounds void (%class.A*, i32)*, void (%class.A*, i32)** %6, i64 0, !dbg !50
  %8 = load void (%class.A*, i32)*, void (%class.A*, i32)** %7, align 8, !dbg !50
  call void %8(%class.A* %4, i32 12), !dbg !50
  %9 = load %class.A*, %class.A** %2, align 8, !dbg !51
  %10 = bitcast %class.A* %9 to i32 (%class.A*)***, !dbg !52
  %11 = load i32 (%class.A*)**, i32 (%class.A*)*** %10, align 8, !dbg !52
  %12 = getelementptr inbounds i32 (%class.A*)*, i32 (%class.A*)** %11, i64 1, !dbg !52
  %13 = load i32 (%class.A*)*, i32 (%class.A*)** %12, align 8, !dbg !52
  %14 = call i32 %13(%class.A* %9), !dbg !52
  store i32 %14, i32* %3, align 4, !dbg !53
  call void @_Z1hi(i32 14), !dbg !54
  %15 = call i32 @_Z2hhi(i32 15), !dbg !55
  store i32 %15, i32* %3, align 4, !dbg !56
  ret void, !dbg !57
}
; CHECK: define void @_Z3runP1A({0: si32 (...)**}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si32 (...)**}** $2 = allocate {0: si32 (...)**}*, 1, align 8
; CHECK:   si32* $3 = allocate si32, 1, align 4
; CHECK:   store $2, %1, align 8
; CHECK:   {0: si32 (...)**}** %4 = bitcast $2
; CHECK:   {0: si32 (...)**}* %5 = load %4, align 8
; CHECK:   void ({0: si32 (...)**}*, si32)*** %6 = bitcast %5
; CHECK:   void ({0: si32 (...)**}*, si32)** %7 = load %6, align 8
; CHECK:   void ({0: si32 (...)**}*, si32)** %8 = ptrshift %7, 8 * 0
; CHECK:   void ({0: si32 (...)**}*, si32)* %9 = load %8, align 8
; CHECK:   call %9(%5, 12)
; CHECK:   {0: si32 (...)**}** %10 = bitcast $2
; CHECK:   {0: si32 (...)**}* %11 = load %10, align 8
; CHECK:   si32 ({0: si32 (...)**}*)*** %12 = bitcast %11
; CHECK:   si32 ({0: si32 (...)**}*)** %13 = load %12, align 8
; CHECK:   si32 ({0: si32 (...)**}*)** %14 = ptrshift %13, 8 * 1
; CHECK:   si32 ({0: si32 (...)**}*)* %15 = load %14, align 8
; CHECK:   si32 %16 = call %15(%11)
; CHECK:   store $3, %16, align 4
; CHECK:   call @_Z1hi(14)
; CHECK:   si32 %17 = call @_Z2hhi(15)
; CHECK:   store $3, %17, align 4
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1A1fEi(%class.A*, i32) unnamed_addr #0 align 2 !dbg !135 {
  %3 = alloca %class.A*, align 8
  %4 = alloca i32, align 4
  store %class.A* %0, %class.A** %3, align 8
  call void @llvm.dbg.declare(metadata %class.A** %3, metadata !136, metadata !DIExpression()), !dbg !137
  store i32 %1, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !138, metadata !DIExpression()), !dbg !139
  %5 = load %class.A*, %class.A** %3, align 8
  ret void, !dbg !140
}
; CHECK: define void @_ZN1A1fEi({0: si32 (...)**}* %1, si32 %2) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si32 (...)**}** $3 = allocate {0: si32 (...)**}*, 1, align 8
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   store $3, %1, align 8
; CHECK:   store $4, %2, align 4
; CHECK:   {0: si32 (...)**}** %5 = bitcast $3
; CHECK:   {0: si32 (...)**}* %6 = load %5, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr i32 @_ZN1A1gEv(%class.A*) unnamed_addr #0 align 2 !dbg !141 {
  %2 = alloca %class.A*, align 8
  store %class.A* %0, %class.A** %2, align 8
  call void @llvm.dbg.declare(metadata %class.A** %2, metadata !142, metadata !DIExpression()), !dbg !143
  %3 = load %class.A*, %class.A** %2, align 8
  ret i32 0, !dbg !144
}
; CHECK: define si32 @_ZN1A1gEv({0: si32 (...)**}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si32 (...)**}** $2 = allocate {0: si32 (...)**}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: si32 (...)**}** %3 = bitcast $2
; CHECK:   {0: si32 (...)**}* %4 = load %3, align 8
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1AC2Ev(%class.A*) unnamed_addr #0 align 2 !dbg !116 {
  %2 = alloca %class.A*, align 8
  store %class.A* %0, %class.A** %2, align 8
  call void @llvm.dbg.declare(metadata %class.A** %2, metadata !120, metadata !DIExpression()), !dbg !121
  %3 = load %class.A*, %class.A** %2, align 8
  %4 = bitcast %class.A* %3 to i32 (...)***, !dbg !122
  %5 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1A, i32 0, i32 0, i32 2, !dbg !122
  %6 = bitcast i8** %5 to i32 (...)**, !dbg !122
  store i32 (...)** %6, i32 (...)*** %4, align 8, !dbg !122
  ret void, !dbg !122
}
; CHECK: define void @_ZN1AC2Ev({0: si32 (...)**}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si32 (...)**}** $2 = allocate {0: si32 (...)**}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: si32 (...)**}** %3 = bitcast $2
; CHECK:   {0: si32 (...)**}* %4 = load %3, align 8
; CHECK:   si32 (...)*** %5 = bitcast %4
; CHECK:   si8** %6 = ptrshift @_ZTV1A, 32 * 0, 1 * 0, 8 * 2
; CHECK:   si32 (...)** %7 = bitcast %6
; CHECK:   store %5, %7, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1B1fEi(%class.B*, i32) unnamed_addr #0 align 2 !dbg !123 {
  %3 = alloca %class.B*, align 8
  %4 = alloca i32, align 4
  store %class.B* %0, %class.B** %3, align 8
  call void @llvm.dbg.declare(metadata %class.B** %3, metadata !124, metadata !DIExpression()), !dbg !125
  store i32 %1, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !126, metadata !DIExpression()), !dbg !127
  %5 = load %class.B*, %class.B** %3, align 8
  %6 = load i32, i32* %4, align 4, !dbg !128
  store i32 %6, i32* @G, align 4, !dbg !129
  ret void, !dbg !130
}
; CHECK: define void @_ZN1B1fEi({0: {0: si32 (...)**}}* %1, si32 %2) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: si32 (...)**}}** $3 = allocate {0: {0: si32 (...)**}}*, 1, align 8
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   store $3, %1, align 8
; CHECK:   store $4, %2, align 4
; CHECK:   {0: {0: si32 (...)**}}** %5 = bitcast $3
; CHECK:   {0: {0: si32 (...)**}}* %6 = load %5, align 8
; CHECK:   si32 %7 = load $4, align 4
; CHECK:   store @G, %7, align 4
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr i32 @_ZN1B1gEv(%class.B*) unnamed_addr #0 align 2 !dbg !131 {
  %2 = alloca %class.B*, align 8
  store %class.B* %0, %class.B** %2, align 8
  call void @llvm.dbg.declare(metadata %class.B** %2, metadata !132, metadata !DIExpression()), !dbg !133
  %3 = load %class.B*, %class.B** %2, align 8
  ret i32 0, !dbg !134
}
; CHECK: define si32 @_ZN1B1gEv({0: {0: si32 (...)**}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: si32 (...)**}}** $2 = allocate {0: {0: si32 (...)**}}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: {0: si32 (...)**}}** %3 = bitcast $2
; CHECK:   {0: {0: si32 (...)**}}* %4 = load %3, align 8
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1BC1Ev(%class.B*) unnamed_addr #0 align 2 !dbg !96 {
  %2 = alloca %class.B*, align 8
  store %class.B* %0, %class.B** %2, align 8
  call void @llvm.dbg.declare(metadata %class.B** %2, metadata !100, metadata !DIExpression()), !dbg !102
  %3 = load %class.B*, %class.B** %2, align 8
  call void @_ZN1BC2Ev(%class.B* %3) #5, !dbg !103
  ret void, !dbg !103
}
; CHECK: define void @_ZN1BC1Ev({0: {0: si32 (...)**}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: si32 (...)**}}** $2 = allocate {0: {0: si32 (...)**}}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: {0: si32 (...)**}}* %3 = load $2, align 8
; CHECK:   call @_ZN1BC2Ev(%3)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1BC2Ev(%class.B*) unnamed_addr #0 align 2 !dbg !112 {
  %2 = alloca %class.B*, align 8
  store %class.B* %0, %class.B** %2, align 8
  call void @llvm.dbg.declare(metadata %class.B** %2, metadata !113, metadata !DIExpression()), !dbg !114
  %3 = load %class.B*, %class.B** %2, align 8
  %4 = bitcast %class.B* %3 to %class.A*, !dbg !115
  call void @_ZN1AC2Ev(%class.A* %4) #5, !dbg !115
  %5 = bitcast %class.B* %3 to i32 (...)***, !dbg !115
  %6 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1B, i32 0, i32 0, i32 2, !dbg !115
  %7 = bitcast i8** %6 to i32 (...)**, !dbg !115
  store i32 (...)** %7, i32 (...)*** %5, align 8, !dbg !115
  ret void, !dbg !115
}
; CHECK: define void @_ZN1BC2Ev({0: {0: si32 (...)**}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: si32 (...)**}}** $2 = allocate {0: {0: si32 (...)**}}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: {0: si32 (...)**}}** %3 = bitcast $2
; CHECK:   {0: {0: si32 (...)**}}* %4 = load %3, align 8
; CHECK:   {0: si32 (...)**}* %5 = bitcast %4
; CHECK:   call @_ZN1AC2Ev(%5)
; CHECK:   si32 (...)*** %6 = bitcast %4
; CHECK:   si8** %7 = ptrshift @_ZTV1B, 32 * 0, 1 * 0, 8 * 2
; CHECK:   si32 (...)** %8 = bitcast %7
; CHECK:   store %6, %8, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1C1fEi(%class.C*, i32) unnamed_addr #0 align 2 !dbg !149 {
  %3 = alloca %class.C*, align 8
  %4 = alloca i32, align 4
  store %class.C* %0, %class.C** %3, align 8
  call void @llvm.dbg.declare(metadata %class.C** %3, metadata !150, metadata !DIExpression()), !dbg !151
  store i32 %1, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !152, metadata !DIExpression()), !dbg !153
  %5 = load %class.C*, %class.C** %3, align 8
  %6 = load i32, i32* %4, align 4, !dbg !154
  %7 = sub nsw i32 0, %6, !dbg !155
  store i32 %7, i32* @G, align 4, !dbg !156
  ret void, !dbg !157
}
; CHECK: define void @_ZN1C1fEi({0: {0: {0: si32 (...)**}}}* %1, si32 %2) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: {0: si32 (...)**}}}** $3 = allocate {0: {0: {0: si32 (...)**}}}*, 1, align 8
; CHECK:   si32* $4 = allocate si32, 1, align 4
; CHECK:   store $3, %1, align 8
; CHECK:   store $4, %2, align 4
; CHECK:   {0: {0: {0: si32 (...)**}}}** %5 = bitcast $3
; CHECK:   {0: {0: {0: si32 (...)**}}}* %6 = load %5, align 8
; CHECK:   si32 %7 = load $4, align 4
; CHECK:   si32 %8 = 0 ssub.nw %7
; CHECK:   store @G, %8, align 4
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr i32 @_ZN1C1gEv(%class.C*) unnamed_addr #0 align 2 !dbg !158 {
  %2 = alloca %class.C*, align 8
  store %class.C* %0, %class.C** %2, align 8
  call void @llvm.dbg.declare(metadata %class.C** %2, metadata !159, metadata !DIExpression()), !dbg !160
  %3 = load %class.C*, %class.C** %2, align 8
  ret i32 1, !dbg !161
}
; CHECK: define si32 @_ZN1C1gEv({0: {0: {0: si32 (...)**}}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: {0: si32 (...)**}}}** $2 = allocate {0: {0: {0: si32 (...)**}}}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: {0: {0: si32 (...)**}}}** %3 = bitcast $2
; CHECK:   {0: {0: {0: si32 (...)**}}}* %4 = load %3, align 8
; CHECK:   return 1
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1CC1Ev(%class.C*) unnamed_addr #0 align 2 !dbg !104 {
  %2 = alloca %class.C*, align 8
  store %class.C* %0, %class.C** %2, align 8
  call void @llvm.dbg.declare(metadata %class.C** %2, metadata !108, metadata !DIExpression()), !dbg !110
  %3 = load %class.C*, %class.C** %2, align 8
  call void @_ZN1CC2Ev(%class.C* %3) #5, !dbg !111
  ret void, !dbg !111
}
; CHECK: define void @_ZN1CC1Ev({0: {0: {0: si32 (...)**}}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: {0: si32 (...)**}}}** $2 = allocate {0: {0: {0: si32 (...)**}}}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: {0: {0: si32 (...)**}}}* %3 = load $2, align 8
; CHECK:   call @_ZN1CC2Ev(%3)
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1CC2Ev(%class.C*) unnamed_addr #0 align 2 !dbg !145 {
  %2 = alloca %class.C*, align 8
  store %class.C* %0, %class.C** %2, align 8
  call void @llvm.dbg.declare(metadata %class.C** %2, metadata !146, metadata !DIExpression()), !dbg !147
  %3 = load %class.C*, %class.C** %2, align 8
  %4 = bitcast %class.C* %3 to %class.B*, !dbg !148
  call void @_ZN1BC2Ev(%class.B* %4) #5, !dbg !148
  %5 = bitcast %class.C* %3 to i32 (...)***, !dbg !148
  %6 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1C, i32 0, i32 0, i32 2, !dbg !148
  %7 = bitcast i8** %6 to i32 (...)**, !dbg !148
  store i32 (...)** %7, i32 (...)*** %5, align 8, !dbg !148
  ret void, !dbg !148
}
; CHECK: define void @_ZN1CC2Ev({0: {0: {0: si32 (...)**}}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   {0: {0: {0: si32 (...)**}}}** $2 = allocate {0: {0: {0: si32 (...)**}}}*, 1, align 8
; CHECK:   store $2, %1, align 8
; CHECK:   {0: {0: {0: si32 (...)**}}}** %3 = bitcast $2
; CHECK:   {0: {0: {0: si32 (...)**}}}* %4 = load %3, align 8
; CHECK:   {0: {0: si32 (...)**}}* %5 = bitcast %4
; CHECK:   call @_ZN1BC2Ev(%5)
; CHECK:   si32 (...)*** %6 = bitcast %4
; CHECK:   si8** %7 = ptrshift @_ZTV1C, 32 * 0, 1 * 0, 8 * 2
; CHECK:   si32 (...)** %8 = bitcast %7
; CHECK:   store %6, %8, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

declare i32 @__gxx_personality_v0(...)
; CHECK: declare si32 @__gxx_personality_v0(...)

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(i8*) #4
; CHECK: declare si32 @ar.eh.typeid.for(si8*)

declare i8* @__cxa_begin_catch(i8*)
; CHECK: declare si8* @ar.libcpp.begincatch(si8*)

declare void @__cxa_end_catch()
; CHECK: declare void @ar.libcpp.endcatch()

; Function Attrs: noinline norecurse ssp uwtable
define i32 @main() #3 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) !dbg !58 {
  %1 = alloca i32, align 4
  %2 = alloca %class.B, align 8
  %3 = alloca %class.C, align 8
  %4 = alloca i8*
  %5 = alloca i32
  %6 = alloca %class.A*, align 8
  store i32 0, i32* %1, align 4
  call void @llvm.dbg.declare(metadata %class.B* %2, metadata !59, metadata !DIExpression()), !dbg !70
  call void @_ZN1BC1Ev(%class.B* %2) #5, !dbg !70
  call void @llvm.dbg.declare(metadata %class.C* %3, metadata !71, metadata !DIExpression()), !dbg !82
  call void @_ZN1CC1Ev(%class.C* %3) #5, !dbg !82
  %7 = bitcast %class.B* %2 to %class.A*, !dbg !83
  invoke void @_Z3runP1A(%class.A* %7)
          to label %8 unwind label %11, !dbg !85

8:                                                ; preds = %0
  %9 = bitcast %class.C* %3 to %class.A*, !dbg !86
  invoke void @_Z3runP1A(%class.A* %9)
          to label %10 unwind label %11, !dbg !87

10:                                               ; preds = %8
  br label %24, !dbg !88

11:                                               ; preds = %8, %0
  %12 = landingpad { i8*, i32 }
          catch i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), !dbg !89
  %13 = extractvalue { i8*, i32 } %12, 0, !dbg !89
  store i8* %13, i8** %4, align 8, !dbg !89
  %14 = extractvalue { i8*, i32 } %12, 1, !dbg !89
  store i32 %14, i32* %5, align 4, !dbg !89
  br label %15, !dbg !89

15:                                               ; preds = %11
  %16 = load i32, i32* %5, align 4, !dbg !88
  %17 = bitcast { i8*, i8* }* @_ZTI1A to i8*, !dbg !88
  %18 = call i32 @llvm.eh.typeid.for(i8* %17) #5, !dbg !88
  %19 = icmp eq i32 %16, %18, !dbg !88
  br i1 %19, label %20, label %25, !dbg !88

20:                                               ; preds = %15
  call void @llvm.dbg.declare(metadata %class.A** %6, metadata !90, metadata !DIExpression()), !dbg !92
  %21 = load i8*, i8** %4, align 8, !dbg !88
  %22 = call i8* @__cxa_begin_catch(i8* %21) #5, !dbg !88
  %23 = bitcast i8* %22 to %class.A*, !dbg !88
  store %class.A* %23, %class.A** %6, align 8, !dbg !88
  call void @__cxa_end_catch(), !dbg !93
  br label %24, !dbg !93

24:                                               ; preds = %20, %10
  ret i32 0, !dbg !95

25:                                               ; preds = %15
  %26 = load i8*, i8** %4, align 8, !dbg !88
  %27 = load i32, i32* %5, align 4, !dbg !88
  %28 = insertvalue { i8*, i32 } undef, i8* %26, 0, !dbg !88
  %29 = insertvalue { i8*, i32 } %28, i32 %27, 1, !dbg !88
  resume { i8*, i32 } %29, !dbg !88
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry successors={#2, #3} {
; CHECK:   si32* $1 = allocate si32, 1, align 4
; CHECK:   {0: {0: si32 (...)**}}* $2 = allocate {0: {0: si32 (...)**}}, 1, align 8
; CHECK:   {0: {0: {0: si32 (...)**}}}* $3 = allocate {0: {0: {0: si32 (...)**}}}, 1, align 8
; CHECK:   si8** $4 = allocate si8*, 1
; CHECK:   si32* $5 = allocate si32, 1
; CHECK:   {0: si32 (...)**}** $6 = allocate {0: si32 (...)**}*, 1, align 8
; CHECK:   store $1, 0, align 4
; CHECK:   call @_ZN1BC1Ev($2)
; CHECK:   call @_ZN1CC1Ev($3)
; CHECK:   {0: si32 (...)**}* %7 = bitcast $2
; CHECK:   invoke @_Z3runP1A(%7) normal=#2 exc=#3
; CHECK: }
; CHECK: #2 predecessors={#1} successors={#4, #5} {
; CHECK:   {0: si32 (...)**}* %8 = bitcast $3
; CHECK:   invoke @_Z3runP1A(%8) normal=#4 exc=#5
; CHECK: }
; CHECK: #3 predecessors={#1} successors={#6} {
; CHECK: }
; CHECK: #4 predecessors={#2} successors={#7} {
; CHECK: }
; CHECK: #5 predecessors={#2} successors={#6} {
; CHECK: }
; CHECK: #6 predecessors={#3, #5} successors={#8, #9} {
; CHECK:   {0: si8*, 8: si32} %9 = landingpad
; CHECK:   si8* %10 = extractelement %9, 0
; CHECK:   store $4, %10, align 8
; CHECK:   si32 %11 = extractelement %9, 8
; CHECK:   store $5, %11, align 4
; CHECK:   si32 %12 = load $5, align 4
; CHECK:   si8* %13 = bitcast @_ZTI1A
; CHECK:   si32 %14 = call @ar.eh.typeid.for(%13)
; CHECK: }
; CHECK: #8 predecessors={#6} successors={#7} {
; CHECK:   %12 sieq %14
; CHECK:   si8* %15 = load $4, align 8
; CHECK:   si8* %16 = call @ar.libcpp.begincatch(%15)
; CHECK:   {0: si32 (...)**}* %17 = bitcast %16
; CHECK:   store $6, %17, align 8
; CHECK:   call @ar.libcpp.endcatch()
; CHECK: }
; CHECK: #9 predecessors={#6} successors={#unified-exit} {
; CHECK:   %12 sine %14
; CHECK:   si8* %18 = load $4, align 8
; CHECK:   si32 %19 = load $5, align 4
; CHECK:   {0: si8*, 8: si32} %20 = insertelement undef, 0, %18
; CHECK:   {0: si8*, 8: si32} %21 = insertelement %20, 8, %19
; CHECK:   resume %21
; CHECK: }
; CHECK: #7 predecessors={#4, #8} successors={#unified-exit} {
; CHECK:   return 0
; CHECK: }
; CHECK: #unified-exit !exit predecessors={#7, #9} {
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { noinline ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noinline norecurse ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind readnone }
attributes #5 = { nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!7, !8, !9, !10}
!llvm.ident = !{!11}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "G", scope: !2, file: !3, line: 1, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !3, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5, nameTableKind: GNU)
!3 = !DIFile(filename: "try-catch.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/no_optimization")
!4 = !{}
!5 = !{!0}
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{i32 2, !"Dwarf Version", i32 4}
!8 = !{i32 2, !"Debug Info Version", i32 3}
!9 = !{i32 1, !"wchar_size", i32 4}
!10 = !{i32 7, !"PIC Level", i32 2}
!11 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!12 = distinct !DISubprogram(name: "h", linkageName: "_Z1hi", scope: !3, file: !3, line: 21, type: !13, scopeLine: 21, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!13 = !DISubroutineType(types: !14)
!14 = !{null, !6}
!15 = !DILocalVariable(name: "x", arg: 1, scope: !12, file: !3, line: 21, type: !6)
!16 = !DILocation(line: 21, column: 12, scope: !12)
!17 = !DILocation(line: 21, column: 16, scope: !12)
!18 = distinct !DISubprogram(name: "hh", linkageName: "_Z2hhi", scope: !3, file: !3, line: 23, type: !19, scopeLine: 23, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!19 = !DISubroutineType(types: !20)
!20 = !{!6, !6}
!21 = !DILocalVariable(name: "x", arg: 1, scope: !18, file: !3, line: 23, type: !6)
!22 = !DILocation(line: 23, column: 12, scope: !18)
!23 = !DILocation(line: 24, column: 10, scope: !18)
!24 = !DILocation(line: 24, column: 14, scope: !18)
!25 = !DILocation(line: 24, column: 12, scope: !18)
!26 = !DILocation(line: 24, column: 3, scope: !18)
!27 = distinct !DISubprogram(name: "run", linkageName: "_Z3runP1A", scope: !3, file: !3, line: 27, type: !28, scopeLine: 27, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!28 = !DISubroutineType(types: !29)
!29 = !{null, !30}
!30 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !31, size: 64)
!31 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "A", file: !3, line: 3, size: 64, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !32, vtableHolder: !31, identifier: "_ZTS1A")
!32 = !{!33, !38, !42}
!33 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$A", scope: !3, file: !3, baseType: !34, size: 64, flags: DIFlagArtificial)
!34 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !35, size: 64)
!35 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "__vtbl_ptr_type", baseType: !36, size: 64)
!36 = !DISubroutineType(types: !37)
!37 = !{!6}
!38 = !DISubprogram(name: "f", linkageName: "_ZN1A1fEi", scope: !31, file: !3, line: 5, type: !39, scopeLine: 5, containingType: !31, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!39 = !DISubroutineType(types: !40)
!40 = !{null, !41, !6}
!41 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !31, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!42 = !DISubprogram(name: "g", linkageName: "_ZN1A1gEv", scope: !31, file: !3, line: 6, type: !43, scopeLine: 6, containingType: !31, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!43 = !DISubroutineType(types: !44)
!44 = !{!6, !41}
!45 = !DILocalVariable(name: "p", arg: 1, scope: !27, file: !3, line: 27, type: !30)
!46 = !DILocation(line: 27, column: 13, scope: !27)
!47 = !DILocalVariable(name: "x", scope: !27, file: !3, line: 28, type: !6)
!48 = !DILocation(line: 28, column: 7, scope: !27)
!49 = !DILocation(line: 29, column: 3, scope: !27)
!50 = !DILocation(line: 29, column: 6, scope: !27)
!51 = !DILocation(line: 30, column: 7, scope: !27)
!52 = !DILocation(line: 30, column: 10, scope: !27)
!53 = !DILocation(line: 30, column: 5, scope: !27)
!54 = !DILocation(line: 31, column: 3, scope: !27)
!55 = !DILocation(line: 32, column: 7, scope: !27)
!56 = !DILocation(line: 32, column: 5, scope: !27)
!57 = !DILocation(line: 33, column: 1, scope: !27)
!58 = distinct !DISubprogram(name: "main", scope: !3, file: !3, line: 35, type: !36, scopeLine: 35, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, retainedNodes: !4)
!59 = !DILocalVariable(name: "b", scope: !58, file: !3, line: 36, type: !60)
!60 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "B", file: !3, line: 9, size: 64, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !61, vtableHolder: !31, identifier: "_ZTS1B")
!61 = !{!62, !63, !67}
!62 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !60, baseType: !31, flags: DIFlagPublic, extraData: i32 0)
!63 = !DISubprogram(name: "f", linkageName: "_ZN1B1fEi", scope: !60, file: !3, line: 11, type: !64, scopeLine: 11, containingType: !60, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!64 = !DISubroutineType(types: !65)
!65 = !{null, !66, !6}
!66 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !60, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!67 = !DISubprogram(name: "g", linkageName: "_ZN1B1gEv", scope: !60, file: !3, line: 12, type: !68, scopeLine: 12, containingType: !60, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!68 = !DISubroutineType(types: !69)
!69 = !{!6, !66}
!70 = !DILocation(line: 36, column: 5, scope: !58)
!71 = !DILocalVariable(name: "c", scope: !58, file: !3, line: 37, type: !72)
!72 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "C", file: !3, line: 15, size: 64, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !73, vtableHolder: !31, identifier: "_ZTS1C")
!73 = !{!74, !75, !79}
!74 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !72, baseType: !60, flags: DIFlagPublic, extraData: i32 0)
!75 = !DISubprogram(name: "f", linkageName: "_ZN1C1fEi", scope: !72, file: !3, line: 17, type: !76, scopeLine: 17, containingType: !72, virtualIndex: 0, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!76 = !DISubroutineType(types: !77)
!77 = !{null, !78, !6}
!78 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !72, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!79 = !DISubprogram(name: "g", linkageName: "_ZN1C1gEv", scope: !72, file: !3, line: 18, type: !80, scopeLine: 18, containingType: !72, virtualIndex: 1, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagVirtual)
!80 = !DISubroutineType(types: !81)
!81 = !{!6, !78}
!82 = !DILocation(line: 37, column: 5, scope: !58)
!83 = !DILocation(line: 39, column: 9, scope: !84)
!84 = distinct !DILexicalBlock(scope: !58, file: !3, line: 38, column: 7)
!85 = !DILocation(line: 39, column: 5, scope: !84)
!86 = !DILocation(line: 40, column: 9, scope: !84)
!87 = !DILocation(line: 40, column: 5, scope: !84)
!88 = !DILocation(line: 41, column: 3, scope: !84)
!89 = !DILocation(line: 44, column: 1, scope: !84)
!90 = !DILocalVariable(name: "e", scope: !58, file: !3, line: 41, type: !91)
!91 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !31, size: 64)
!92 = !DILocation(line: 41, column: 15, scope: !58)
!93 = !DILocation(line: 42, column: 3, scope: !94)
!94 = distinct !DILexicalBlock(scope: !58, file: !3, line: 41, column: 18)
!95 = !DILocation(line: 43, column: 3, scope: !58)
!96 = distinct !DISubprogram(name: "B", linkageName: "_ZN1BC1Ev", scope: !60, file: !3, line: 9, type: !97, scopeLine: 9, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !99, retainedNodes: !4)
!97 = !DISubroutineType(types: !98)
!98 = !{null, !66}
!99 = !DISubprogram(name: "B", scope: !60, type: !97, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!100 = !DILocalVariable(name: "this", arg: 1, scope: !96, type: !101, flags: DIFlagArtificial | DIFlagObjectPointer)
!101 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !60, size: 64)
!102 = !DILocation(line: 0, scope: !96)
!103 = !DILocation(line: 9, column: 7, scope: !96)
!104 = distinct !DISubprogram(name: "C", linkageName: "_ZN1CC1Ev", scope: !72, file: !3, line: 15, type: !105, scopeLine: 15, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !107, retainedNodes: !4)
!105 = !DISubroutineType(types: !106)
!106 = !{null, !78}
!107 = !DISubprogram(name: "C", scope: !72, type: !105, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!108 = !DILocalVariable(name: "this", arg: 1, scope: !104, type: !109, flags: DIFlagArtificial | DIFlagObjectPointer)
!109 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !72, size: 64)
!110 = !DILocation(line: 0, scope: !104)
!111 = !DILocation(line: 15, column: 7, scope: !104)
!112 = distinct !DISubprogram(name: "B", linkageName: "_ZN1BC2Ev", scope: !60, file: !3, line: 9, type: !97, scopeLine: 9, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !99, retainedNodes: !4)
!113 = !DILocalVariable(name: "this", arg: 1, scope: !112, type: !101, flags: DIFlagArtificial | DIFlagObjectPointer)
!114 = !DILocation(line: 0, scope: !112)
!115 = !DILocation(line: 9, column: 7, scope: !112)
!116 = distinct !DISubprogram(name: "A", linkageName: "_ZN1AC2Ev", scope: !31, file: !3, line: 3, type: !117, scopeLine: 3, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !119, retainedNodes: !4)
!117 = !DISubroutineType(types: !118)
!118 = !{null, !41}
!119 = !DISubprogram(name: "A", scope: !31, type: !117, flags: DIFlagPublic | DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!120 = !DILocalVariable(name: "this", arg: 1, scope: !116, type: !30, flags: DIFlagArtificial | DIFlagObjectPointer)
!121 = !DILocation(line: 0, scope: !116)
!122 = !DILocation(line: 3, column: 7, scope: !116)
!123 = distinct !DISubprogram(name: "f", linkageName: "_ZN1B1fEi", scope: !60, file: !3, line: 11, type: !64, scopeLine: 11, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !63, retainedNodes: !4)
!124 = !DILocalVariable(name: "this", arg: 1, scope: !123, type: !101, flags: DIFlagArtificial | DIFlagObjectPointer)
!125 = !DILocation(line: 0, scope: !123)
!126 = !DILocalVariable(name: "x", arg: 2, scope: !123, file: !3, line: 11, type: !6)
!127 = !DILocation(line: 11, column: 22, scope: !123)
!128 = !DILocation(line: 11, column: 31, scope: !123)
!129 = !DILocation(line: 11, column: 29, scope: !123)
!130 = !DILocation(line: 11, column: 34, scope: !123)
!131 = distinct !DISubprogram(name: "g", linkageName: "_ZN1B1gEv", scope: !60, file: !3, line: 12, type: !68, scopeLine: 12, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !67, retainedNodes: !4)
!132 = !DILocalVariable(name: "this", arg: 1, scope: !131, type: !101, flags: DIFlagArtificial | DIFlagObjectPointer)
!133 = !DILocation(line: 0, scope: !131)
!134 = !DILocation(line: 12, column: 21, scope: !131)
!135 = distinct !DISubprogram(name: "f", linkageName: "_ZN1A1fEi", scope: !31, file: !3, line: 5, type: !39, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !38, retainedNodes: !4)
!136 = !DILocalVariable(name: "this", arg: 1, scope: !135, type: !30, flags: DIFlagArtificial | DIFlagObjectPointer)
!137 = !DILocation(line: 0, scope: !135)
!138 = !DILocalVariable(name: "x", arg: 2, scope: !135, file: !3, line: 5, type: !6)
!139 = !DILocation(line: 5, column: 22, scope: !135)
!140 = !DILocation(line: 5, column: 26, scope: !135)
!141 = distinct !DISubprogram(name: "g", linkageName: "_ZN1A1gEv", scope: !31, file: !3, line: 6, type: !43, scopeLine: 6, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !42, retainedNodes: !4)
!142 = !DILocalVariable(name: "this", arg: 1, scope: !141, type: !30, flags: DIFlagArtificial | DIFlagObjectPointer)
!143 = !DILocation(line: 0, scope: !141)
!144 = !DILocation(line: 6, column: 21, scope: !141)
!145 = distinct !DISubprogram(name: "C", linkageName: "_ZN1CC2Ev", scope: !72, file: !3, line: 15, type: !105, scopeLine: 15, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !107, retainedNodes: !4)
!146 = !DILocalVariable(name: "this", arg: 1, scope: !145, type: !109, flags: DIFlagArtificial | DIFlagObjectPointer)
!147 = !DILocation(line: 0, scope: !145)
!148 = !DILocation(line: 15, column: 7, scope: !145)
!149 = distinct !DISubprogram(name: "f", linkageName: "_ZN1C1fEi", scope: !72, file: !3, line: 17, type: !76, scopeLine: 17, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !75, retainedNodes: !4)
!150 = !DILocalVariable(name: "this", arg: 1, scope: !149, type: !109, flags: DIFlagArtificial | DIFlagObjectPointer)
!151 = !DILocation(line: 0, scope: !149)
!152 = !DILocalVariable(name: "x", arg: 2, scope: !149, file: !3, line: 17, type: !6)
!153 = !DILocation(line: 17, column: 22, scope: !149)
!154 = !DILocation(line: 17, column: 32, scope: !149)
!155 = !DILocation(line: 17, column: 31, scope: !149)
!156 = !DILocation(line: 17, column: 29, scope: !149)
!157 = !DILocation(line: 17, column: 35, scope: !149)
!158 = distinct !DISubprogram(name: "g", linkageName: "_ZN1C1gEv", scope: !72, file: !3, line: 18, type: !80, scopeLine: 18, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !2, declaration: !79, retainedNodes: !4)
!159 = !DILocalVariable(name: "this", arg: 1, scope: !158, type: !109, flags: DIFlagArtificial | DIFlagObjectPointer)
!160 = !DILocation(line: 0, scope: !158)
!161 = !DILocation(line: 18, column: 21, scope: !158)
