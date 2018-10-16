; ModuleID = 'virtual-inheritance.cpp.pp.bc'
source_filename = "virtual-inheritance.cpp"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.12.0"

; CHECK-LABEL: Bundle
; CHECK: target-endianness = little-endian
; CHECK: target-pointer-size = 64 bits
; CHECK: target-triple = x86_64-apple-macosx10.12.0

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

@_ZTV1E = internal unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* inttoptr (i64 24 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 }* @_ZTI1E to i8*), i8* bitcast (void (%struct.E*)* @_ZN1E1fEv to i8*)] }, align 8
@_ZTVN10__cxxabiv121__vmi_class_type_infoE = external global i8*
@_ZTS1E = internal constant [3 x i8] c"1E\00"
@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS1A = internal constant [3 x i8] c"1A\00"
@_ZTI1A = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1A, i32 0, i32 0) }
@_ZTS1B = internal constant [3 x i8] c"1B\00"
@_ZTI1B = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1B, i32 0, i32 0) }
@_ZTS1C = internal constant [3 x i8] c"1C\00"
@_ZTI1C = internal constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1C, i32 0, i32 0) }
@_ZTI1E = internal constant { i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1E, i32 0, i32 0), i32 0, i32 3, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141, i8* bitcast ({ i8*, i8* }* @_ZTI1B to i8*), i64 2050, i8* bitcast ({ i8*, i8* }* @_ZTI1C to i8*), i64 4098 }
@_ZTV1D = internal unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* inttoptr (i64 24 to i8*), i8* inttoptr (i64 16 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 }* @_ZTI1D to i8*)] }, align 8
@_ZTS1D = internal constant [3 x i8] c"1D\00"
@_ZTI1D = internal constant { i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1D, i32 0, i32 0), i32 0, i32 3, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141, i8* bitcast ({ i8*, i8* }* @_ZTI1B to i8*), i64 -8189, i8* bitcast ({ i8*, i8* }* @_ZTI1C to i8*), i64 2050 }
@_ZTV1F = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 12 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1F to i8*)] }, align 8
@_ZTS1F = internal constant [3 x i8] c"1F\00"
@_ZTI1F = internal constant { i8*, i8*, i32, i32, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1F, i32 0, i32 0), i32 0, i32 1, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141 }
@_ZTV1G = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 16 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1G to i8*)] }, align 8
@_ZTS1G = internal constant [3 x i8] c"1G\00"
@_ZTI1G = internal constant { i8*, i8*, i32, i32, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1G, i32 0, i32 0), i32 0, i32 1, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141 }
@_ZTV1H = internal unnamed_addr constant { [3 x i8*], [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 32 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64 }* @_ZTI1H to i8*)], [3 x i8*] [i8* inttoptr (i64 16 to i8*), i8* inttoptr (i64 -16 to i8*), i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64 }* @_ZTI1H to i8*)] }, align 8
@_ZTT1H = internal unnamed_addr constant [4 x i8*] [i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i32 0, inrange i32 0, i32 3) to i8*), i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTC1H0_1F, i32 0, inrange i32 0, i32 3) to i8*), i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTC1H16_1G, i32 0, inrange i32 0, i32 3) to i8*), i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i32 0, inrange i32 1, i32 3) to i8*)]
@_ZTC1H0_1F = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 32 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1F to i8*)] }
@_ZTC1H16_1G = internal unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 16 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1G to i8*)] }
@_ZTS1H = internal constant [3 x i8] c"1H\00"
@_ZTI1H = internal constant { i8*, i8*, i32, i32, i8*, i64, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1H, i32 0, i32 0), i32 2, i32 2, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1F to i8*), i64 2, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1G to i8*), i64 4098 }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1DC1Ev(%struct.D*) unnamed_addr #1 align 2 !dbg !77 {
  call void @llvm.dbg.value(metadata %struct.D* %0, i64 0, metadata !82, metadata !74), !dbg !84
  %2 = getelementptr inbounds %struct.D, %struct.D* %0, i64 0, i32 0, !dbg !85
  %3 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1D, i64 1, i32 0, i64 0, !dbg !85
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !85
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !85
  ret void, !dbg !85
}
; CHECK: define void @_ZN1DC1Ev({0: si32 (...)**, 8: {0: ui8}, 12: float, 16: {0: si32}, 24: {0: si8*}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = ptrshift %1, 32 * 0, 1 * 0
; CHECK:   si8** %3 = ptrshift @_ZTV1D, 32 * 1, 1 * 0, 8 * 0
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal void @_ZN1E1fEv(%struct.E*) unnamed_addr #1 align 2 !dbg !115 {
  call void @llvm.dbg.value(metadata %struct.E* %0, i64 0, metadata !116, metadata !74), !dbg !117
  ret void, !dbg !118
}
; CHECK: define void @_ZN1E1fEv({0: si32 (...)**, 8: {0: si8*}, 16: {0: ui8}, 20: si32, 24: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1EC1Ev(%struct.E*) unnamed_addr #1 align 2 !dbg !70 {
  call void @llvm.dbg.value(metadata %struct.E* %0, i64 0, metadata !72, metadata !74), !dbg !75
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
  call void @llvm.dbg.value(metadata %struct.F* %0, i64 0, metadata !91, metadata !74), !dbg !93
  %2 = getelementptr inbounds %struct.F, %struct.F* %0, i64 0, i32 0, !dbg !94
  %3 = getelementptr inbounds { [3 x i8*] }, { [3 x i8*] }* @_ZTV1F, i64 1, i32 0, i64 0, !dbg !94
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !94
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !94
  ret void, !dbg !94
}
; CHECK: define void @_ZN1FC1Ev(<{0: si32 (...)**, 8: si32, 12: {0: si32}}>* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = ptrshift %1, 16 * 0, 1 * 0
; CHECK:   si8** %3 = ptrshift @_ZTV1F, 24 * 1, 1 * 0, 8 * 0
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1FC2Ev(%struct.F*, i8**) unnamed_addr #1 align 2 !dbg !119 {
  call void @llvm.dbg.value(metadata %struct.F* %0, i64 0, metadata !120, metadata !74), !dbg !121
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !122, metadata !74), !dbg !121
  %3 = bitcast i8** %1 to i64*, !dbg !125
  %4 = load i64, i64* %3, align 8, !dbg !125
  %5 = bitcast %struct.F* %0 to i64*, !dbg !125
  store i64 %4, i64* %5, align 8, !dbg !125
  ret void, !dbg !125
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
  call void @llvm.dbg.value(metadata %struct.G* %0, i64 0, metadata !100, metadata !74), !dbg !102
  %2 = getelementptr inbounds %struct.G, %struct.G* %0, i64 0, i32 0, !dbg !103
  %3 = getelementptr inbounds { [3 x i8*] }, { [3 x i8*] }* @_ZTV1G, i64 1, i32 0, i64 0, !dbg !103
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !103
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !103
  ret void, !dbg !103
}
; CHECK: define void @_ZN1GC1Ev({0: si32 (...)**, 8: si8*, 16: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = ptrshift %1, 24 * 0, 1 * 0
; CHECK:   si8** %3 = ptrshift @_ZTV1G, 24 * 1, 1 * 0, 8 * 0
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define internal fastcc void @_ZN1GC2Ev(%struct.G*, i8**) unnamed_addr #1 align 2 !dbg !126 {
  call void @llvm.dbg.value(metadata %struct.G* %0, i64 0, metadata !127, metadata !74), !dbg !128
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !129, metadata !74), !dbg !128
  %3 = bitcast i8** %1 to i64*, !dbg !130
  %4 = load i64, i64* %3, align 8, !dbg !130
  %5 = bitcast %struct.G* %0 to i64*, !dbg !130
  store i64 %4, i64* %5, align 8, !dbg !130
  ret void, !dbg !130
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
  call void @llvm.dbg.value(metadata %struct.H* %0, i64 0, metadata !109, metadata !74), !dbg !111
  %2 = bitcast %struct.H* %0 to %struct.F*, !dbg !112
  %3 = getelementptr inbounds [4 x i8*], [4 x i8*]* @_ZTT1H, i64 0, i64 1, !dbg !112
  call fastcc void @_ZN1FC2Ev(%struct.F* %2, i8** %3) #3, !dbg !112
  %4 = getelementptr inbounds %struct.H, %struct.H* %0, i64 0, i32 1, !dbg !112
  %5 = bitcast %struct.G.base* %4 to %struct.G*, !dbg !112
  %6 = getelementptr inbounds [4 x i8*], [4 x i8*]* @_ZTT1H, i64 0, i64 2, !dbg !113
  call fastcc void @_ZN1GC2Ev(%struct.G* %5, i8** %6) #3, !dbg !113
  %7 = getelementptr inbounds %struct.H, %struct.H* %0, i64 0, i32 0, i32 0, !dbg !112
  %8 = getelementptr inbounds { [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i64 0, i32 1, i64 0, !dbg !112
  %9 = bitcast i8** %8 to i32 (...)**, !dbg !112
  store i32 (...)** %9, i32 (...)*** %7, align 8, !dbg !112
  %10 = getelementptr inbounds %struct.G.base, %struct.G.base* %4, i64 0, i32 0, !dbg !112
  %11 = getelementptr inbounds { [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i64 1, i32 0, i64 0, !dbg !112
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
; CHECK:   si8** %8 = ptrshift @_ZTV1H, 48 * 0, 1 * 24, 8 * 0
; CHECK:   si32 (...)** %9 = bitcast %8
; CHECK:   store %7, %9, align 8
; CHECK:   si32 (...)*** %10 = ptrshift %4, 16 * 0, 1 * 0
; CHECK:   si8** %11 = ptrshift @_ZTV1H, 48 * 1, 1 * 0, 8 * 0
; CHECK:   si32 (...)** %12 = bitcast %11
; CHECK:   store %10, %12, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline norecurse nounwind ssp uwtable
define i32 @main() local_unnamed_addr #0 !dbg !7 {
  %1 = alloca %struct.E, align 8
  %2 = alloca %struct.D, align 8
  %3 = alloca %struct.F, align 8
  %4 = alloca %struct.G, align 8
  %5 = alloca %struct.H, align 8
  call void @llvm.dbg.value(metadata %struct.E* %1, i64 0, metadata !11, metadata !37), !dbg !38
  call fastcc void @_ZN1EC1Ev(%struct.E* nonnull %1) #3, !dbg !38
  call void @llvm.dbg.value(metadata %struct.D* %2, i64 0, metadata !39, metadata !37), !dbg !48
  call fastcc void @_ZN1DC1Ev(%struct.D* nonnull %2) #3, !dbg !48
  call void @llvm.dbg.value(metadata %struct.F* %3, i64 0, metadata !49, metadata !37), !dbg !55
  call fastcc void @_ZN1FC1Ev(%struct.F* nonnull %3) #3, !dbg !55
  call void @llvm.dbg.value(metadata %struct.G* %4, i64 0, metadata !56, metadata !37), !dbg !62
  call fastcc void @_ZN1GC1Ev(%struct.G* nonnull %4) #3, !dbg !62
  call void @llvm.dbg.value(metadata %struct.H* %5, i64 0, metadata !63, metadata !37), !dbg !68
  call fastcc void @_ZN1HC1Ev(%struct.H* nonnull %5) #3, !dbg !68
  ret i32 0, !dbg !69
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

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #2

attributes #0 = { noinline norecurse nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "virtual-inheritance.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/aggressive_optimization")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"PIC Level", i32 2}
!6 = !{!"clang version 4.0.1 (tags/RELEASE_401/final)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 33, type: !8, isLocal: false, isDefinition: true, scopeLine: 33, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocalVariable(name: "e", scope: !7, file: !1, line: 34, type: !12)
!12 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "E", file: !1, line: 13, size: 256, elements: !13, vtableHolder: !12, identifier: "_ZTS1E")
!13 = !{!14, !18, !24, !29, !32, !33}
!14 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !12, baseType: !15, offset: 24, flags: DIFlagVirtual)
!15 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "A", file: !1, line: 1, size: 32, elements: !16, identifier: "_ZTS1A")
!16 = !{!17}
!17 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !15, file: !1, line: 2, baseType: !10, size: 32)
!18 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !12, baseType: !19, offset: 64)
!19 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "B", file: !1, line: 5, size: 64, elements: !20, identifier: "_ZTS1B")
!20 = !{!21}
!21 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !19, file: !1, line: 6, baseType: !22, size: 64)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !23, size: 64)
!23 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!24 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !12, baseType: !25, offset: 128)
!25 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "C", file: !1, line: 9, size: 8, elements: !26, identifier: "_ZTS1C")
!26 = !{!27}
!27 = !DIDerivedType(tag: DW_TAG_member, name: "a", scope: !25, file: !1, line: 10, baseType: !28, size: 8)
!28 = !DIBasicType(name: "bool", size: 8, encoding: DW_ATE_boolean)
!29 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$E", scope: !1, file: !1, baseType: !30, size: 64, flags: DIFlagArtificial)
!30 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !31, size: 64)
!31 = !DIDerivedType(tag: DW_TAG_pointer_type, name: "__vtbl_ptr_type", baseType: !8, size: 64)
!32 = !DIDerivedType(tag: DW_TAG_member, name: "x", scope: !12, file: !1, line: 14, baseType: !10, size: 32, offset: 160)
!33 = !DISubprogram(name: "f", linkageName: "_ZN1E1fEv", scope: !12, file: !1, line: 16, type: !34, isLocal: false, isDefinition: false, scopeLine: 16, containingType: !12, virtuality: DW_VIRTUALITY_virtual, virtualIndex: 0, flags: DIFlagPrototyped, isOptimized: false)
!34 = !DISubroutineType(types: !35)
!35 = !{null, !36}
!36 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!37 = !DIExpression(DW_OP_deref)
!38 = !DILocation(line: 34, column: 5, scope: !7)
!39 = !DILocalVariable(name: "d", scope: !7, file: !1, line: 35, type: !40)
!40 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "D", file: !1, line: 19, size: 256, elements: !41, vtableHolder: !40, identifier: "_ZTS1D")
!41 = !{!42, !43, !44, !45, !46}
!42 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !40, baseType: !15, offset: 24, flags: DIFlagVirtual)
!43 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !40, baseType: !19, offset: 32, flags: DIFlagVirtual)
!44 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !40, baseType: !25, offset: 64)
!45 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$D", scope: !1, file: !1, baseType: !30, size: 64, flags: DIFlagArtificial)
!46 = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !40, file: !1, line: 20, baseType: !47, size: 32, offset: 96)
!47 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!48 = !DILocation(line: 35, column: 5, scope: !7)
!49 = !DILocalVariable(name: "f", scope: !7, file: !1, line: 36, type: !50)
!50 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "F", file: !1, line: 23, size: 128, elements: !51, vtableHolder: !50, identifier: "_ZTS1F")
!51 = !{!52, !53, !54}
!52 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !50, baseType: !15, offset: 24, flags: DIFlagVirtual)
!53 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$F", scope: !1, file: !1, baseType: !30, size: 64, flags: DIFlagArtificial)
!54 = !DIDerivedType(tag: DW_TAG_member, name: "y", scope: !50, file: !1, line: 24, baseType: !10, size: 32, offset: 64)
!55 = !DILocation(line: 36, column: 5, scope: !7)
!56 = !DILocalVariable(name: "g", scope: !7, file: !1, line: 37, type: !57)
!57 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "G", file: !1, line: 27, size: 192, elements: !58, vtableHolder: !57, identifier: "_ZTS1G")
!58 = !{!59, !60, !61}
!59 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !57, baseType: !15, offset: 24, flags: DIFlagVirtual)
!60 = !DIDerivedType(tag: DW_TAG_member, name: "_vptr$G", scope: !1, file: !1, baseType: !30, size: 64, flags: DIFlagArtificial)
!61 = !DIDerivedType(tag: DW_TAG_member, name: "z", scope: !57, file: !1, line: 28, baseType: !22, size: 64, offset: 64)
!62 = !DILocation(line: 37, column: 5, scope: !7)
!63 = !DILocalVariable(name: "h", scope: !7, file: !1, line: 38, type: !64)
!64 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "H", file: !1, line: 31, size: 320, elements: !65, vtableHolder: !50, identifier: "_ZTS1H")
!65 = !{!66, !67}
!66 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !64, baseType: !50)
!67 = !DIDerivedType(tag: DW_TAG_inheritance, scope: !64, baseType: !57, offset: 128)
!68 = !DILocation(line: 38, column: 5, scope: !7)
!69 = !DILocation(line: 39, column: 3, scope: !7)
!70 = distinct !DISubprogram(name: "E", linkageName: "_ZN1EC1Ev", scope: !12, file: !1, line: 13, type: !34, isLocal: false, isDefinition: true, scopeLine: 13, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !71, variables: !2)
!71 = !DISubprogram(name: "E", scope: !12, type: !34, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!72 = !DILocalVariable(name: "this", arg: 1, scope: !70, type: !73, flags: DIFlagArtificial | DIFlagObjectPointer)
!73 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !12, size: 64)
!74 = !DIExpression()
!75 = !DILocation(line: 0, scope: !70)
!76 = !DILocation(line: 13, column: 8, scope: !70)
!77 = distinct !DISubprogram(name: "D", linkageName: "_ZN1DC1Ev", scope: !40, file: !1, line: 19, type: !78, isLocal: false, isDefinition: true, scopeLine: 19, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !81, variables: !2)
!78 = !DISubroutineType(types: !79)
!79 = !{null, !80}
!80 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !40, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!81 = !DISubprogram(name: "D", scope: !40, type: !78, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!82 = !DILocalVariable(name: "this", arg: 1, scope: !77, type: !83, flags: DIFlagArtificial | DIFlagObjectPointer)
!83 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !40, size: 64)
!84 = !DILocation(line: 0, scope: !77)
!85 = !DILocation(line: 19, column: 8, scope: !77)
!86 = distinct !DISubprogram(name: "F", linkageName: "_ZN1FC1Ev", scope: !50, file: !1, line: 23, type: !87, isLocal: false, isDefinition: true, scopeLine: 23, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !90, variables: !2)
!87 = !DISubroutineType(types: !88)
!88 = !{null, !89}
!89 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !50, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!90 = !DISubprogram(name: "F", scope: !50, type: !87, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!91 = !DILocalVariable(name: "this", arg: 1, scope: !86, type: !92, flags: DIFlagArtificial | DIFlagObjectPointer)
!92 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !50, size: 64)
!93 = !DILocation(line: 0, scope: !86)
!94 = !DILocation(line: 23, column: 8, scope: !86)
!95 = distinct !DISubprogram(name: "G", linkageName: "_ZN1GC1Ev", scope: !57, file: !1, line: 27, type: !96, isLocal: false, isDefinition: true, scopeLine: 27, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !99, variables: !2)
!96 = !DISubroutineType(types: !97)
!97 = !{null, !98}
!98 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !57, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!99 = !DISubprogram(name: "G", scope: !57, type: !96, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!100 = !DILocalVariable(name: "this", arg: 1, scope: !95, type: !101, flags: DIFlagArtificial | DIFlagObjectPointer)
!101 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !57, size: 64)
!102 = !DILocation(line: 0, scope: !95)
!103 = !DILocation(line: 27, column: 8, scope: !95)
!104 = distinct !DISubprogram(name: "H", linkageName: "_ZN1HC1Ev", scope: !64, file: !1, line: 31, type: !105, isLocal: false, isDefinition: true, scopeLine: 31, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !108, variables: !2)
!105 = !DISubroutineType(types: !106)
!106 = !{null, !107}
!107 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !64, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!108 = !DISubprogram(name: "H", scope: !64, type: !105, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!109 = !DILocalVariable(name: "this", arg: 1, scope: !104, type: !110, flags: DIFlagArtificial | DIFlagObjectPointer)
!110 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !64, size: 64)
!111 = !DILocation(line: 0, scope: !104)
!112 = !DILocation(line: 31, column: 8, scope: !104)
!113 = !DILocation(line: 31, column: 8, scope: !114)
!114 = !DILexicalBlockFile(scope: !104, file: !1, discriminator: 1)
!115 = distinct !DISubprogram(name: "f", linkageName: "_ZN1E1fEv", scope: !12, file: !1, line: 16, type: !34, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !33, variables: !2)
!116 = !DILocalVariable(name: "this", arg: 1, scope: !115, type: !73, flags: DIFlagArtificial | DIFlagObjectPointer)
!117 = !DILocation(line: 0, scope: !115)
!118 = !DILocation(line: 16, column: 21, scope: !115)
!119 = distinct !DISubprogram(name: "F", linkageName: "_ZN1FC2Ev", scope: !50, file: !1, line: 23, type: !87, isLocal: false, isDefinition: true, scopeLine: 23, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !90, variables: !2)
!120 = !DILocalVariable(name: "this", arg: 1, scope: !119, type: !92, flags: DIFlagArtificial | DIFlagObjectPointer)
!121 = !DILocation(line: 0, scope: !119)
!122 = !DILocalVariable(name: "vtt", arg: 2, scope: !119, type: !123, flags: DIFlagArtificial)
!123 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !124, size: 64)
!124 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!125 = !DILocation(line: 23, column: 8, scope: !119)
!126 = distinct !DISubprogram(name: "G", linkageName: "_ZN1GC2Ev", scope: !57, file: !1, line: 27, type: !96, isLocal: false, isDefinition: true, scopeLine: 27, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !99, variables: !2)
!127 = !DILocalVariable(name: "this", arg: 1, scope: !126, type: !101, flags: DIFlagArtificial | DIFlagObjectPointer)
!128 = !DILocation(line: 0, scope: !126)
!129 = !DILocalVariable(name: "vtt", arg: 2, scope: !126, type: !123, flags: DIFlagArtificial)
!130 = !DILocation(line: 27, column: 8, scope: !126)
