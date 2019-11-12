; ModuleID = 'virtual-inheritance.pp.bc'
source_filename = "virtual-inheritance.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.14.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.14.0

%struct.E = type { i32 (...)**, %struct.B, %struct.C, i32, %struct.A }
%struct.B = type { i8* }
%struct.C = type { i8 }
%struct.A = type { i32 }
%struct.D = type { i32 (...)**, %struct.C, float, %struct.A, %struct.B }
%struct.F = type <{ i32 (...)**, i32, %struct.A }>
%struct.G = type { i32 (...)**, i8*, %struct.A }
%struct.H = type { %struct.F.base, %struct.G.base, %struct.A }
%struct.F.base = type <{ i32 (...)**, i32 }>
%struct.G.base = type { i32 (...)**, i8* }

@_ZTC1H0_1F = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 32 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1F to i8*)] }, align 8
; CHECK: define {0: [3 x si8*]}* @_ZTC1H0_1F, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZTI1F
; CHECK:   si8* %2 = sitoptr 32
; CHECK:   store @_ZTC1H0_1F, {0: [%2, null, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTC1H16_1G = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 16 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1G to i8*)] }, align 8
; CHECK: define {0: [3 x si8*]}* @_ZTC1H16_1G, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZTI1G
; CHECK:   si8* %2 = sitoptr 16
; CHECK:   store @_ZTC1H16_1G, {0: [%2, null, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTI1A = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1A, i32 0, i32 0) }, align 8
; CHECK: define {0: si8*, 8: si8*}* @_ZTI1A, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv117__class_type_infoE, 8 * 2
; CHECK:   si8* %2 = ptrshift @_ZTS1A, 3 * 0, 1 * 0
; CHECK:   si8* %3 = bitcast %1
; CHECK:   store @_ZTI1A, {0: %3, 8: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTI1B = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1B, i32 0, i32 0) }, align 8
; CHECK: define {0: si8*, 8: si8*}* @_ZTI1B, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv117__class_type_infoE, 8 * 2
; CHECK:   si8* %2 = ptrshift @_ZTS1B, 3 * 0, 1 * 0
; CHECK:   si8* %3 = bitcast %1
; CHECK:   store @_ZTI1B, {0: %3, 8: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTI1C = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1C, i32 0, i32 0) }, align 8
; CHECK: define {0: si8*, 8: si8*}* @_ZTI1C, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv117__class_type_infoE, 8 * 2
; CHECK:   si8* %2 = ptrshift @_ZTS1C, 3 * 0, 1 * 0
; CHECK:   si8* %3 = bitcast %1
; CHECK:   store @_ZTI1C, {0: %3, 8: %2}, align 1
; CHECK: }
; CHECK: }

@_ZTI1D = internal constant { i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1D, i32 0, i32 0), i32 0, i32 3, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141, i8* bitcast ({ i8*, i8* }* @_ZTI1B to i8*), i64 -8189, i8* bitcast ({ i8*, i8* }* @_ZTI1C to i8*), i64 2050 }, align 8
; CHECK: define {0: si8*, 8: si8*, 16: si32, 20: si32, 24: si8*, 32: si64, 40: si8*, 48: si64, 56: si8*, 64: si64}* @_ZTI1D, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv121__vmi_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1C
; CHECK:   si8* %3 = bitcast @_ZTI1B
; CHECK:   si8* %4 = bitcast @_ZTI1A
; CHECK:   si8* %5 = ptrshift @_ZTS1D, 3 * 0, 1 * 0
; CHECK:   si8* %6 = bitcast %1
; CHECK:   store @_ZTI1D, {0: %6, 8: %5, 16: 0, 20: 3, 24: %4, 32: -6141, 40: %3, 48: -8189, 56: %2, 64: 2050}, align 1
; CHECK: }
; CHECK: }

@_ZTI1E = internal constant { i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1E, i32 0, i32 0), i32 0, i32 3, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141, i8* bitcast ({ i8*, i8* }* @_ZTI1B to i8*), i64 2050, i8* bitcast ({ i8*, i8* }* @_ZTI1C to i8*), i64 4098 }, align 8
; CHECK: define {0: si8*, 8: si8*, 16: si32, 20: si32, 24: si8*, 32: si64, 40: si8*, 48: si64, 56: si8*, 64: si64}* @_ZTI1E, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv121__vmi_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1C
; CHECK:   si8* %3 = bitcast @_ZTI1B
; CHECK:   si8* %4 = bitcast @_ZTI1A
; CHECK:   si8* %5 = ptrshift @_ZTS1E, 3 * 0, 1 * 0
; CHECK:   si8* %6 = bitcast %1
; CHECK:   store @_ZTI1E, {0: %6, 8: %5, 16: 0, 20: 3, 24: %4, 32: -6141, 40: %3, 48: 2050, 56: %2, 64: 4098}, align 1
; CHECK: }
; CHECK: }

@_ZTI1F = internal constant { i8*, i8*, i32, i32, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1F, i32 0, i32 0), i32 0, i32 1, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141 }, align 8
; CHECK: define {0: si8*, 8: si8*, 16: si32, 20: si32, 24: si8*, 32: si64}* @_ZTI1F, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv121__vmi_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1A
; CHECK:   si8* %3 = ptrshift @_ZTS1F, 3 * 0, 1 * 0
; CHECK:   si8* %4 = bitcast %1
; CHECK:   store @_ZTI1F, {0: %4, 8: %3, 16: 0, 20: 1, 24: %2, 32: -6141}, align 1
; CHECK: }
; CHECK: }

@_ZTI1G = internal constant { i8*, i8*, i32, i32, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1G, i32 0, i32 0), i32 0, i32 1, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141 }, align 8
; CHECK: define {0: si8*, 8: si8*, 16: si32, 20: si32, 24: si8*, 32: si64}* @_ZTI1G, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv121__vmi_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1A
; CHECK:   si8* %3 = ptrshift @_ZTS1G, 3 * 0, 1 * 0
; CHECK:   si8* %4 = bitcast %1
; CHECK:   store @_ZTI1G, {0: %4, 8: %3, 16: 0, 20: 1, 24: %2, 32: -6141}, align 1
; CHECK: }
; CHECK: }

@_ZTI1H = internal constant { i8*, i8*, i32, i32, i8*, i64, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1H, i32 0, i32 0), i32 2, i32 2, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1F to i8*), i64 2, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1G to i8*), i64 4098 }, align 8
; CHECK: define {0: si8*, 8: si8*, 16: si32, 20: si32, 24: si8*, 32: si64, 40: si8*, 48: si64}* @_ZTI1H, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTVN10__cxxabiv121__vmi_class_type_infoE, 8 * 2
; CHECK:   si8* %2 = bitcast @_ZTI1G
; CHECK:   si8* %3 = bitcast @_ZTI1F
; CHECK:   si8* %4 = ptrshift @_ZTS1H, 3 * 0, 1 * 0
; CHECK:   si8* %5 = bitcast %1
; CHECK:   store @_ZTI1H, {0: %5, 8: %4, 16: 2, 20: 2, 24: %3, 32: 2, 40: %2, 48: 4098}, align 1
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

@_ZTS1D = internal constant [3 x i8] c"1D\00", align 1
; CHECK: define [3 x si8]* @_ZTS1D, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1D, [49, 68, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1E = internal constant [3 x i8] c"1E\00", align 1
; CHECK: define [3 x si8]* @_ZTS1E, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1E, [49, 69, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1F = internal constant [3 x i8] c"1F\00", align 1
; CHECK: define [3 x si8]* @_ZTS1F, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1F, [49, 70, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1G = internal constant [3 x i8] c"1G\00", align 1
; CHECK: define [3 x si8]* @_ZTS1G, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1G, [49, 71, 0], align 1
; CHECK: }
; CHECK: }

@_ZTS1H = internal constant [3 x i8] c"1H\00", align 1
; CHECK: define [3 x si8]* @_ZTS1H, align 1, init {
; CHECK: #1 !entry !exit {
; CHECK:   store @_ZTS1H, [49, 72, 0], align 1
; CHECK: }
; CHECK: }

@_ZTT1H = internal unnamed_addr constant [4 x i8*] [i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i32 0, inrange i32 0, i32 3) to i8*), i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTC1H0_1F, i32 0, inrange i32 0, i32 3) to i8*), i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTC1H16_1G, i32 0, inrange i32 0, i32 3) to i8*), i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i32 0, inrange i32 1, i32 3) to i8*)], align 8
; CHECK: define [4 x si8*]* @_ZTT1H, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8** %1 = ptrshift @_ZTV1H, 48 * 0, 1 * 24, 8 * 3
; CHECK:   si8** %2 = ptrshift @_ZTC1H16_1G, 24 * 0, 1 * 0, 8 * 3
; CHECK:   si8** %3 = ptrshift @_ZTC1H0_1F, 24 * 0, 1 * 0, 8 * 3
; CHECK:   si8** %4 = ptrshift @_ZTV1H, 48 * 0, 1 * 0, 8 * 3
; CHECK:   si8* %5 = bitcast %1
; CHECK:   si8* %6 = bitcast %2
; CHECK:   si8* %7 = bitcast %3
; CHECK:   si8* %8 = bitcast %4
; CHECK:   store @_ZTT1H, [%8, %7, %6, %5], align 1
; CHECK: }
; CHECK: }

@_ZTV1D = internal unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* inttoptr (i64 24 to i8*), i8* inttoptr (i64 16 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 }* @_ZTI1D to i8*)] }, align 8
; CHECK: define {0: [4 x si8*]}* @_ZTV1D, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZTI1D
; CHECK:   si8* %2 = sitoptr 16
; CHECK:   si8* %3 = sitoptr 24
; CHECK:   store @_ZTV1D, {0: [%3, %2, null, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTV1E = internal unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* inttoptr (i64 24 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 }* @_ZTI1E to i8*), i8* bitcast (void (%struct.E*)* @_ZN1E1fEv to i8*)] }, align 8
; CHECK: define {0: [4 x si8*]}* @_ZTV1E, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZN1E1fEv
; CHECK:   si8* %2 = bitcast @_ZTI1E
; CHECK:   si8* %3 = sitoptr 24
; CHECK:   store @_ZTV1E, {0: [%3, null, %2, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTV1F = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 12 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1F to i8*)] }, align 8
; CHECK: define {0: [3 x si8*]}* @_ZTV1F, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZTI1F
; CHECK:   si8* %2 = sitoptr 12
; CHECK:   store @_ZTV1F, {0: [%2, null, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTV1G = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 16 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1G to i8*)] }, align 8
; CHECK: define {0: [3 x si8*]}* @_ZTV1G, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZTI1G
; CHECK:   si8* %2 = sitoptr 16
; CHECK:   store @_ZTV1G, {0: [%2, null, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTV1H = internal unnamed_addr constant { [3 x i8*], [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 32 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64 }* @_ZTI1H to i8*)], [3 x i8*] [i8* inttoptr (i64 16 to i8*), i8* inttoptr (i64 -16 to i8*), i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64 }* @_ZTI1H to i8*)] }, align 8
; CHECK: define {0: [3 x si8*], 24: [3 x si8*]}* @_ZTV1H, align 8, init {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %1 = bitcast @_ZTI1H
; CHECK:   si8* %2 = sitoptr -16
; CHECK:   si8* %3 = sitoptr 16
; CHECK:   si8* %4 = bitcast @_ZTI1H
; CHECK:   si8* %5 = sitoptr 32
; CHECK:   store @_ZTV1H, {0: [%5, null, %4], 24: [%3, %2, %1]}, align 1
; CHECK: }
; CHECK: }

@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
; CHECK: declare si8** @_ZTVN10__cxxabiv117__class_type_infoE

@_ZTVN10__cxxabiv121__vmi_class_type_infoE = external global i8*
; CHECK: declare si8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1DC1Ev(%struct.D*) unnamed_addr #1 align 2 !dbg !77 {
  call void @llvm.dbg.value(metadata %struct.D* %0, metadata !82, metadata !DIExpression()), !dbg !84
  %2 = getelementptr inbounds %struct.D, %struct.D* %0, i64 0, i32 0, !dbg !85
  %3 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1D, i64 0, i32 0, i64 4, !dbg !85
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !85
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !85
  ret void, !dbg !85
}
; CHECK: define void @_ZN1DC1Ev({0: si32 (...)**, 8: {0: ui8}, 12: float, 16: {0: si32}, 24: {0: si8*}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = ptrshift %1, 32 * 0, 1 * 0
; CHECK:   si8** %3 = ptrshift @_ZTV1D, 32 * 0, 1 * 0, 8 * 4
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal void @_ZN1E1fEv(%struct.E*) unnamed_addr #1 align 2 !dbg !113 {
  call void @llvm.dbg.value(metadata %struct.E* %0, metadata !114, metadata !DIExpression()), !dbg !115
  ret void, !dbg !116
}
; CHECK: define void @_ZN1E1fEv({0: si32 (...)**, 8: {0: si8*}, 16: {0: ui8}, 20: si32, 24: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1EC1Ev(%struct.E*) unnamed_addr #1 align 2 !dbg !71 {
  call void @llvm.dbg.value(metadata %struct.E* %0, metadata !73, metadata !DIExpression()), !dbg !75
  %2 = getelementptr inbounds %struct.E, %struct.E* %0, i64 0, i32 0, !dbg !76
  %3 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1E, i64 0, i32 0, i64 3, !dbg !76
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !76
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !76
  ret void, !dbg !76
}
; CHECK: define void @_ZN1EC1Ev({0: si32 (...)**, 8: {0: si8*}, 16: {0: ui8}, 20: si32, 24: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = ptrshift %1, 32 * 0, 1 * 0
; CHECK:   si8** %3 = ptrshift @_ZTV1E, 32 * 0, 1 * 0, 8 * 3
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1FC1Ev(%struct.F*) unnamed_addr #1 align 2 !dbg !86 {
  call void @llvm.dbg.value(metadata %struct.F* %0, metadata !91, metadata !DIExpression()), !dbg !93
  %2 = getelementptr inbounds %struct.F, %struct.F* %0, i64 0, i32 0, !dbg !94
  %3 = getelementptr inbounds { [3 x i8*] }, { [3 x i8*] }* @_ZTV1F, i64 0, i32 0, i64 3, !dbg !94
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !94
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !94
  ret void, !dbg !94
}
; CHECK: define void @_ZN1FC1Ev(<{0: si32 (...)**, 8: si32, 12: {0: si32}}>* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = ptrshift %1, 16 * 0, 1 * 0
; CHECK:   si8** %3 = ptrshift @_ZTV1F, 24 * 0, 1 * 0, 8 * 3
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1FC2Ev(%struct.F*, i8**) unnamed_addr #1 align 2 !dbg !117 {
  call void @llvm.dbg.value(metadata %struct.F* %0, metadata !118, metadata !DIExpression()), !dbg !119
  call void @llvm.dbg.value(metadata i8** %1, metadata !120, metadata !DIExpression()), !dbg !119
  %3 = bitcast i8** %1 to i64*, !dbg !123
  %4 = load i64, i64* %3, align 8, !dbg !123
  %5 = bitcast %struct.F* %0 to i64*, !dbg !123
  store i64 %4, i64* %5, align 8, !dbg !123
  ret void, !dbg !123
}
; CHECK: define void @_ZN1FC2Ev(<{0: si32 (...)**, 8: si32, 12: {0: si32}}>* %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si64* %3 = bitcast %2
; CHECK:   si64 %4 = load %3, align 8
; CHECK:   si64* %5 = bitcast %1
; CHECK:   store %5, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1GC1Ev(%struct.G*) unnamed_addr #1 align 2 !dbg !95 {
  call void @llvm.dbg.value(metadata %struct.G* %0, metadata !100, metadata !DIExpression()), !dbg !102
  %2 = getelementptr inbounds %struct.G, %struct.G* %0, i64 0, i32 0, !dbg !103
  %3 = getelementptr inbounds { [3 x i8*] }, { [3 x i8*] }* @_ZTV1G, i64 0, i32 0, i64 3, !dbg !103
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !103
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !103
  ret void, !dbg !103
}
; CHECK: define void @_ZN1GC1Ev({0: si32 (...)**, 8: si8*, 16: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = ptrshift %1, 24 * 0, 1 * 0
; CHECK:   si8** %3 = ptrshift @_ZTV1G, 24 * 0, 1 * 0, 8 * 3
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1GC2Ev(%struct.G*, i8**) unnamed_addr #1 align 2 !dbg !124 {
  call void @llvm.dbg.value(metadata %struct.G* %0, metadata !125, metadata !DIExpression()), !dbg !126
  call void @llvm.dbg.value(metadata i8** %1, metadata !127, metadata !DIExpression()), !dbg !126
  %3 = bitcast i8** %1 to i64*, !dbg !128
  %4 = load i64, i64* %3, align 8, !dbg !128
  %5 = bitcast %struct.G* %0 to i64*, !dbg !128
  store i64 %4, i64* %5, align 8, !dbg !128
  ret void, !dbg !128
}
; CHECK: define void @_ZN1GC2Ev({0: si32 (...)**, 8: si8*, 16: {0: si32}}* %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si64* %3 = bitcast %2
; CHECK:   si64 %4 = load %3, align 8
; CHECK:   si64* %5 = bitcast %1
; CHECK:   store %5, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1HC1Ev(%struct.H*) unnamed_addr #1 align 2 !dbg !104 {
  call void @llvm.dbg.value(metadata %struct.H* %0, metadata !109, metadata !DIExpression()), !dbg !111
  %2 = bitcast %struct.H* %0 to %struct.F*, !dbg !112
  %3 = getelementptr inbounds [4 x i8*], [4 x i8*]* @_ZTT1H, i64 0, i64 1, !dbg !112
  call fastcc void @_ZN1FC2Ev(%struct.F* %2, i8** %3) #3, !dbg !112
  %4 = getelementptr inbounds %struct.H, %struct.H* %0, i64 0, i32 1, !dbg !112
  %5 = bitcast %struct.G.base* %4 to %struct.G*, !dbg !112
  %6 = getelementptr inbounds [4 x i8*], [4 x i8*]* @_ZTT1H, i64 0, i64 2, !dbg !112
  call fastcc void @_ZN1GC2Ev(%struct.G* nonnull %5, i8** %6) #3, !dbg !112
  %7 = getelementptr inbounds %struct.H, %struct.H* %0, i64 0, i32 0, i32 0, !dbg !112
  %8 = getelementptr inbounds { [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i64 0, i32 0, i64 3, !dbg !112
  %9 = bitcast i8** %8 to i32 (...)**, !dbg !112
  store i32 (...)** %9, i32 (...)*** %7, align 8, !dbg !112
  %10 = getelementptr inbounds %struct.G.base, %struct.G.base* %4, i64 0, i32 0, !dbg !112
  %11 = getelementptr inbounds { [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i64 0, i32 1, i64 3, !dbg !112
  %12 = bitcast i8** %11 to i32 (...)**, !dbg !112
  store i32 (...)** %12, i32 (...)*** %10, align 8, !dbg !112
  ret void, !dbg !112
}
; CHECK: define void @_ZN1HC1Ev({0: <{0: si32 (...)**, 8: si32}>, 16: {0: si32 (...)**, 8: si8*}, 32: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   <{0: si32 (...)**, 8: si32, 12: {0: si32}}>* %2 = bitcast %1
; CHECK:   si8** %3 = ptrshift @_ZTT1H, 32 * 0, 8 * 1
; CHECK:   call @_ZN1FC2Ev(%2, %3)
; CHECK:   {0: si32 (...)**, 8: si8*}* %4 = ptrshift %1, 40 * 0, 1 * 16
; CHECK:   {0: si32 (...)**, 8: si8*, 16: {0: si32}}* %5 = bitcast %4
; CHECK:   si8** %6 = ptrshift @_ZTT1H, 32 * 0, 8 * 2
; CHECK:   call @_ZN1GC2Ev(%5, %6)
; CHECK:   si32 (...)*** %7 = ptrshift %1, 40 * 0, 1 * 0, 1 * 0
; CHECK:   si8** %8 = ptrshift @_ZTV1H, 48 * 0, 1 * 0, 8 * 3
; CHECK:   si32 (...)** %9 = bitcast %8
; CHECK:   store %7, %9, align 8
; CHECK:   si32 (...)*** %10 = ptrshift %4, 16 * 0, 1 * 0
; CHECK:   si8** %11 = ptrshift @_ZTV1H, 48 * 0, 1 * 24, 8 * 3
; CHECK:   si32 (...)** %12 = bitcast %11
; CHECK:   store %10, %12, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline norecurse nounwind ssp uwtable
define i32 @main() local_unnamed_addr #0 !dbg !8 {
  %1 = alloca %struct.E, align 8
  %2 = alloca %struct.D, align 8
  %3 = alloca %struct.F, align 8
  %4 = alloca %struct.G, align 8
  %5 = alloca %struct.H, align 8
  call void @llvm.dbg.value(metadata %struct.E* %1, metadata !12, metadata !DIExpression(DW_OP_deref)), !dbg !38
  call fastcc void @_ZN1EC1Ev(%struct.E* nonnull %1) #3, !dbg !39
  call void @llvm.dbg.value(metadata %struct.D* %2, metadata !40, metadata !DIExpression(DW_OP_deref)), !dbg !38
  call fastcc void @_ZN1DC1Ev(%struct.D* nonnull %2) #3, !dbg !49
  call void @llvm.dbg.value(metadata %struct.F* %3, metadata !50, metadata !DIExpression(DW_OP_deref)), !dbg !38
  call fastcc void @_ZN1FC1Ev(%struct.F* nonnull %3) #3, !dbg !56
  call void @llvm.dbg.value(metadata %struct.G* %4, metadata !57, metadata !DIExpression(DW_OP_deref)), !dbg !38
  call fastcc void @_ZN1GC1Ev(%struct.G* nonnull %4) #3, !dbg !63
  call void @llvm.dbg.value(metadata %struct.H* %5, metadata !64, metadata !DIExpression(DW_OP_deref)), !dbg !38
  call fastcc void @_ZN1HC1Ev(%struct.H* nonnull %5) #3, !dbg !69
  ret i32 0, !dbg !70
}
; CHECK: define si32 @main() {
; CHECK: #1 !entry !exit {
; CHECK:   {0: si32 (...)**, 8: {0: si8*}, 16: {0: ui8}, 20: si32, 24: {0: si32}}* $1 = allocate {0: si32 (...)**, 8: {0: si8*}, 16: {0: ui8}, 20: si32, 24: {0: si32}}, 1, align 8
; CHECK:   {0: si32 (...)**, 8: {0: ui8}, 12: float, 16: {0: si32}, 24: {0: si8*}}* $2 = allocate {0: si32 (...)**, 8: {0: ui8}, 12: float, 16: {0: si32}, 24: {0: si8*}}, 1, align 8
; CHECK:   <{0: si32 (...)**, 8: si32, 12: {0: si32}}>* $3 = allocate <{0: si32 (...)**, 8: si32, 12: {0: si32}}>, 1, align 8
; CHECK:   {0: si32 (...)**, 8: si8*, 16: {0: si32}}* $4 = allocate {0: si32 (...)**, 8: si8*, 16: {0: si32}}, 1, align 8
; CHECK:   {0: <{0: si32 (...)**, 8: si32}>, 16: {0: si32 (...)**, 8: si8*}, 32: {0: si32}}* $5 = allocate {0: <{0: si32 (...)**, 8: si32}>, 16: {0: si32 (...)**, 8: si8*}, 32: {0: si32}}, 1, align 8
; CHECK:   call @_ZN1EC1Ev($1)
; CHECK:   call @_ZN1DC1Ev($2)
; CHECK:   call @_ZN1FC1Ev($3)
; CHECK:   call @_ZN1GC1Ev($4)
; CHECK:   call @_ZN1HC1Ev($5)
; CHECK:   return 0
; CHECK: }
; CHECK: }

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.value(metadata, metadata, metadata) #2

attributes #0 = { noinline norecurse nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+cx8,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5, !6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 9.0.0 (tags/RELEASE_900/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: GNU)
!1 = !DIFile(filename: "virtual-inheritance.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"PIC Level", i32 2}
!7 = !{!"clang version 9.0.0 (tags/RELEASE_900/final)"}
!8 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 33, type: !9, scopeLine: 33, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{!11}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DILocalVariable(name: "e", scope: !8, file: !1, line: 34, type: !13)
!13 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "E", file: !1, line: 13, size: 256, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !14, vtableHolder: !13, identifier: "_ZTS1E")
!14 = !{!15, !19, !25, !30, !33, !34}
!15 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !13, baseType: !16, offset: 24, flags: DIFlagVirtual, extraData: i32 0)
!16 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "A", file: !1, line: 1, size: 32, flags: DIFlagTypePassByValue, elements: !17, identifier: "_ZTS1A")
!17 = !{!18}
!18 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !16, file: !1, line: 2, baseType: !11, size: 32)
!19 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !13, baseType: !20, offset: 64, extraData: i32 0)
!20 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "B", file: !1, line: 5, size: 64, flags: DIFlagTypePassByValue, elements: !21, identifier: "_ZTS1B")
!21 = !{!22}
!22 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !20, file: !1, line: 6, baseType: !23, size: 64)
!23 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !24, size: 64)
!24 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!25 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !13, baseType: !26, offset: 128, extraData: i32 0)
!26 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "C", file: !1, line: 9, size: 8, flags: DIFlagTypePassByValue, elements: !27, identifier: "_ZTS1C")
!27 = !{!28}
!28 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !26, file: !1, line: 10, baseType: !29, size: 8)
!29 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!30 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$E", scope: !1, file: !1, baseType: !31, size: 64, flags: DIFlagArtificial)
!31 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !32, size: 64)
!32 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "__vtbl_ptr_type", baseType: !9, size: 64)
!33 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !13, file: !1, line: 14, baseType: !11, size: 32, offset: 160)
!34 = !DISubprogram(name: "f", linkageName: "_ZN1E1fEv", scope: !13, file: !1, line: 16, type: !35, scopeLine: 16, containingType: !13, virtualIndex: 0, flags: DIFlagPrototyped, spFlags: DISPFlagVirtual)
!35 = !DISubroutineType(types: !36)
!36 = !{null, !37}
!37 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!38 = !DILocation(line: 0, scope: !8)
!39 = !DILocation(line: 34, column: 5, scope: !8)
!40 = !DILocalVariable(name: "d", scope: !8, file: !1, line: 35, type: !41)
!41 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "D", file: !1, line: 19, size: 256, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !42, vtableHolder: !41, identifier: "_ZTS1D")
!42 = !{!43, !44, !45, !46, !47}
!43 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !41, baseType: !16, offset: 24, flags: DIFlagVirtual, extraData: i32 0)
!44 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !41, baseType: !20, offset: 32, flags: DIFlagVirtual, extraData: i32 0)
!45 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !41, baseType: !26, offset: 64, extraData: i32 0)
!46 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$D", scope: !1, file: !1, baseType: !31, size: 64, flags: DIFlagArtificial)
!47 = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !41, file: !1, line: 20, baseType: !48, size: 32, offset: 96)
!48 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!49 = !DILocation(line: 35, column: 5, scope: !8)
!50 = !DILocalVariable(name: "f", scope: !8, file: !1, line: 36, type: !51)
!51 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "F", file: !1, line: 23, size: 128, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !52, vtableHolder: !51, identifier: "_ZTS1F")
!52 = !{!53, !54, !55}
!53 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !51, baseType: !16, offset: 24, flags: DIFlagVirtual, extraData: i32 0)
!54 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$F", scope: !1, file: !1, baseType: !31, size: 64, flags: DIFlagArtificial)
!55 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !51, file: !1, line: 24, baseType: !11, size: 32, offset: 64)
!56 = !DILocation(line: 36, column: 5, scope: !8)
!57 = !DILocalVariable(name: "g", scope: !8, file: !1, line: 37, type: !58)
!58 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "G", file: !1, line: 27, size: 192, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !59, vtableHolder: !58, identifier: "_ZTS1G")
!59 = !{!60, !61, !62}
!60 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !58, baseType: !16, offset: 24, flags: DIFlagVirtual, extraData: i32 0)
!61 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$G", scope: !1, file: !1, baseType: !31, size: 64, flags: DIFlagArtificial)
!62 = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !58, file: !1, line: 28, baseType: !23, size: 64, offset: 64)
!63 = !DILocation(line: 37, column: 5, scope: !8)
!64 = !DILocalVariable(name: "h", scope: !8, file: !1, line: 38, type: !65)
!65 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "H", file: !1, line: 31, size: 320, flags: DIFlagTypePassByReference | DIFlagNonTrivial, elements: !66, vtableHolder: !51, identifier: "_ZTS1H")
!66 = !{!67, !68}
!67 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !65, baseType: !51, extraData: i32 0)
!68 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !65, baseType: !58, offset: 128, extraData: i32 0)
!69 = !DILocation(line: 38, column: 5, scope: !8)
!70 = !DILocation(line: 39, column: 3, scope: !8)
!71 = distinct !DISubprogram(name: "E", linkageName: "_ZN1EC1Ev", scope: !13, file: !1, line: 13, type: !35, scopeLine: 13, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !72, retainedNodes: !2)
!72 = !DISubprogram(name: "E", scope: !13, type: !35, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!73 = !DILocalVariable(name: "this", arg: 1, scope: !71, type: !74, flags: DIFlagArtificial | DIFlagObjectPointer)
!74 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !13, size: 64)
!75 = !DILocation(line: 0, scope: !71)
!76 = !DILocation(line: 13, column: 8, scope: !71)
!77 = distinct !DISubprogram(name: "D", linkageName: "_ZN1DC1Ev", scope: !41, file: !1, line: 19, type: !78, scopeLine: 19, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !81, retainedNodes: !2)
!78 = !DISubroutineType(types: !79)
!79 = !{null, !80}
!80 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !41, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!81 = !DISubprogram(name: "D", scope: !41, type: !78, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!82 = !DILocalVariable(name: "this", arg: 1, scope: !77, type: !83, flags: DIFlagArtificial | DIFlagObjectPointer)
!83 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !41, size: 64)
!84 = !DILocation(line: 0, scope: !77)
!85 = !DILocation(line: 19, column: 8, scope: !77)
!86 = distinct !DISubprogram(name: "F", linkageName: "_ZN1FC1Ev", scope: !51, file: !1, line: 23, type: !87, scopeLine: 23, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !90, retainedNodes: !2)
!87 = !DISubroutineType(types: !88)
!88 = !{null, !89}
!89 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !51, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!90 = !DISubprogram(name: "F", scope: !51, type: !87, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!91 = !DILocalVariable(name: "this", arg: 1, scope: !86, type: !92, flags: DIFlagArtificial | DIFlagObjectPointer)
!92 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !51, size: 64)
!93 = !DILocation(line: 0, scope: !86)
!94 = !DILocation(line: 23, column: 8, scope: !86)
!95 = distinct !DISubprogram(name: "G", linkageName: "_ZN1GC1Ev", scope: !58, file: !1, line: 27, type: !96, scopeLine: 27, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !99, retainedNodes: !2)
!96 = !DISubroutineType(types: !97)
!97 = !{null, !98}
!98 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !58, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!99 = !DISubprogram(name: "G", scope: !58, type: !96, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!100 = !DILocalVariable(name: "this", arg: 1, scope: !95, type: !101, flags: DIFlagArtificial | DIFlagObjectPointer)
!101 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !58, size: 64)
!102 = !DILocation(line: 0, scope: !95)
!103 = !DILocation(line: 27, column: 8, scope: !95)
!104 = distinct !DISubprogram(name: "H", linkageName: "_ZN1HC1Ev", scope: !65, file: !1, line: 31, type: !105, scopeLine: 31, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !108, retainedNodes: !2)
!105 = !DISubroutineType(types: !106)
!106 = !{null, !107}
!107 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !65, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!108 = !DISubprogram(name: "H", scope: !65, type: !105, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: 0)
!109 = !DILocalVariable(name: "this", arg: 1, scope: !104, type: !110, flags: DIFlagArtificial | DIFlagObjectPointer)
!110 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !65, size: 64)
!111 = !DILocation(line: 0, scope: !104)
!112 = !DILocation(line: 31, column: 8, scope: !104)
!113 = distinct !DISubprogram(name: "f", linkageName: "_ZN1E1fEv", scope: !13, file: !1, line: 16, type: !35, scopeLine: 16, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !34, retainedNodes: !2)
!114 = !DILocalVariable(name: "this", arg: 1, scope: !113, type: !74, flags: DIFlagArtificial | DIFlagObjectPointer)
!115 = !DILocation(line: 0, scope: !113)
!116 = !DILocation(line: 16, column: 21, scope: !113)
!117 = distinct !DISubprogram(name: "F", linkageName: "_ZN1FC2Ev", scope: !51, file: !1, line: 23, type: !87, scopeLine: 23, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !90, retainedNodes: !2)
!118 = !DILocalVariable(name: "this", arg: 1, scope: !117, type: !92, flags: DIFlagArtificial | DIFlagObjectPointer)
!119 = !DILocation(line: 0, scope: !117)
!120 = !DILocalVariable(name: "vtt", arg: 2, scope: !117, type: !121, flags: DIFlagArtificial)
!121 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !122, size: 64)
!122 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!123 = !DILocation(line: 23, column: 8, scope: !117)
!124 = distinct !DISubprogram(name: "G", linkageName: "_ZN1GC2Ev", scope: !58, file: !1, line: 27, type: !96, scopeLine: 27, flags: DIFlagArtificial | DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, declaration: !99, retainedNodes: !2)
!125 = !DILocalVariable(name: "this", arg: 1, scope: !124, type: !101, flags: DIFlagArtificial | DIFlagObjectPointer)
!126 = !DILocation(line: 0, scope: !124)
!127 = !DILocalVariable(name: "vtt", arg: 2, scope: !124, type: !121, flags: DIFlagArtificial)
!128 = !DILocation(line: 27, column: 8, scope: !124)
