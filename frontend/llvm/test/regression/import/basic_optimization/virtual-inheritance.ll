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

@_ZTV1E = linkonce_odr unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* inttoptr (i64 24 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 }* @_ZTI1E to i8*), i8* bitcast (void (%struct.E*)* @_ZN1E1fEv to i8*)] }, align 8
@_ZTVN10__cxxabiv121__vmi_class_type_infoE = external global i8*
@_ZTS1E = linkonce_odr constant [3 x i8] c"1E\00"
@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS1A = linkonce_odr constant [3 x i8] c"1A\00"
@_ZTI1A = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1A, i32 0, i32 0) }
@_ZTS1B = linkonce_odr constant [3 x i8] c"1B\00"
@_ZTI1B = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1B, i32 0, i32 0) }
@_ZTS1C = linkonce_odr constant [3 x i8] c"1C\00"
@_ZTI1C = linkonce_odr constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1C, i32 0, i32 0) }
@_ZTI1E = linkonce_odr constant { i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1E, i32 0, i32 0), i32 0, i32 3, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141, i8* bitcast ({ i8*, i8* }* @_ZTI1B to i8*), i64 2050, i8* bitcast ({ i8*, i8* }* @_ZTI1C to i8*), i64 4098 }
@_ZTV1D = linkonce_odr unnamed_addr constant { [4 x i8*] } { [4 x i8*] [i8* inttoptr (i64 24 to i8*), i8* inttoptr (i64 16 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 }* @_ZTI1D to i8*)] }, align 8
@_ZTS1D = linkonce_odr constant [3 x i8] c"1D\00"
@_ZTI1D = linkonce_odr constant { i8*, i8*, i32, i32, i8*, i64, i8*, i64, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1D, i32 0, i32 0), i32 0, i32 3, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141, i8* bitcast ({ i8*, i8* }* @_ZTI1B to i8*), i64 -8189, i8* bitcast ({ i8*, i8* }* @_ZTI1C to i8*), i64 2050 }
@_ZTV1F = linkonce_odr unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 12 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1F to i8*)] }, align 8
@_ZTS1F = linkonce_odr constant [3 x i8] c"1F\00"
@_ZTI1F = linkonce_odr constant { i8*, i8*, i32, i32, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1F, i32 0, i32 0), i32 0, i32 1, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141 }
@_ZTV1G = linkonce_odr unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 16 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1G to i8*)] }, align 8
@_ZTS1G = linkonce_odr constant [3 x i8] c"1G\00"
@_ZTI1G = linkonce_odr constant { i8*, i8*, i32, i32, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1G, i32 0, i32 0), i32 0, i32 1, i8* bitcast ({ i8*, i8* }* @_ZTI1A to i8*), i64 -6141 }
@_ZTV1H = linkonce_odr unnamed_addr constant { [3 x i8*], [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 32 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64 }* @_ZTI1H to i8*)], [3 x i8*] [i8* inttoptr (i64 16 to i8*), i8* inttoptr (i64 -16 to i8*), i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64, i8*, i64 }* @_ZTI1H to i8*)] }, align 8
@_ZTT1H = linkonce_odr unnamed_addr constant [4 x i8*] [i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i32 0, inrange i32 0, i32 3) to i8*), i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTC1H0_1F, i32 0, inrange i32 0, i32 3) to i8*), i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTC1H16_1G, i32 0, inrange i32 0, i32 3) to i8*), i8* bitcast (i8** getelementptr inbounds ({ [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i32 0, inrange i32 1, i32 3) to i8*)]
@_ZTC1H0_1F = linkonce_odr unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 32 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1F to i8*)] }
@_ZTC1H16_1G = linkonce_odr unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* inttoptr (i64 16 to i8*), i8* null, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1G to i8*)] }
@_ZTS1H = linkonce_odr constant [3 x i8] c"1H\00"
@_ZTI1H = linkonce_odr constant { i8*, i8*, i32, i32, i8*, i64, i8*, i64 } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([3 x i8], [3 x i8]* @_ZTS1H, i32 0, i32 0), i32 2, i32 2, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1F to i8*), i64 2, i8* bitcast ({ i8*, i8*, i32, i32, i8*, i64 }* @_ZTI1G to i8*), i64 4098 }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1DC1Ev(%struct.D*) unnamed_addr #2 align 2 !dbg !76 {
  call void @llvm.dbg.value(metadata %struct.D* %0, i64 0, metadata !81, metadata !37), !dbg !83
  %2 = bitcast %struct.D* %0 to i32 (...)***, !dbg !84
  %3 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1D, i32 0, i32 0, i32 4, !dbg !84
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !84
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !84
  ret void, !dbg !84
}
; CHECK: define void @_ZN1DC1Ev({0: si32 (...)**, 8: {0: ui8}, 12: float, 16: {0: si32}, 24: {0: si8*}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = bitcast %1
; CHECK:   si8** %3 = ptrshift @_ZTV1D, 32 * 0, 1 * 0, 8 * 4
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1E1fEv(%struct.E*) unnamed_addr #2 align 2 !dbg !114 {
  call void @llvm.dbg.value(metadata %struct.E* %0, i64 0, metadata !115, metadata !37), !dbg !116
  ret void, !dbg !117
}
; CHECK: define void @_ZN1E1fEv({0: si32 (...)**, 8: {0: si8*}, 16: {0: ui8}, 20: si32, 24: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1EC1Ev(%struct.E*) unnamed_addr #2 align 2 !dbg !70 {
  call void @llvm.dbg.value(metadata %struct.E* %0, i64 0, metadata !72, metadata !37), !dbg !74
  %2 = bitcast %struct.E* %0 to i32 (...)***, !dbg !75
  %3 = getelementptr inbounds { [4 x i8*] }, { [4 x i8*] }* @_ZTV1E, i32 0, i32 0, i32 3, !dbg !75
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !75
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !75
  ret void, !dbg !75
}
; CHECK: define void @_ZN1EC1Ev({0: si32 (...)**, 8: {0: si8*}, 16: {0: ui8}, 20: si32, 24: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = bitcast %1
; CHECK:   si8** %3 = ptrshift @_ZTV1E, 32 * 0, 1 * 0, 8 * 3
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1FC1Ev(%struct.F*) unnamed_addr #2 align 2 !dbg !85 {
  call void @llvm.dbg.value(metadata %struct.F* %0, i64 0, metadata !90, metadata !37), !dbg !92
  %2 = bitcast %struct.F* %0 to i32 (...)***, !dbg !93
  %3 = getelementptr inbounds { [3 x i8*] }, { [3 x i8*] }* @_ZTV1F, i32 0, i32 0, i32 3, !dbg !93
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !93
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !93
  ret void, !dbg !93
}
; CHECK: define void @_ZN1FC1Ev(<{0: si32 (...)**, 8: si32, 12: {0: si32}}>* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = bitcast %1
; CHECK:   si8** %3 = ptrshift @_ZTV1F, 24 * 0, 1 * 0, 8 * 3
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1FC2Ev(%struct.F*, i8**) unnamed_addr #2 align 2 !dbg !118 {
  call void @llvm.dbg.value(metadata %struct.F* %0, i64 0, metadata !119, metadata !37), !dbg !120
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !121, metadata !37), !dbg !120
  %3 = load i8*, i8** %1, align 8, !dbg !124
  %4 = bitcast %struct.F* %0 to i32 (...)***, !dbg !124
  %5 = bitcast i8* %3 to i32 (...)**, !dbg !124
  store i32 (...)** %5, i32 (...)*** %4, align 8, !dbg !124
  ret void, !dbg !124
}
; CHECK: define void @_ZN1FC2Ev(<{0: si32 (...)**, 8: si32, 12: {0: si32}}>* %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %3 = load %2, align 8
; CHECK:   si32 (...)*** %4 = bitcast %1
; CHECK:   si32 (...)** %5 = bitcast %3
; CHECK:   store %4, %5, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1GC1Ev(%struct.G*) unnamed_addr #2 align 2 !dbg !94 {
  call void @llvm.dbg.value(metadata %struct.G* %0, i64 0, metadata !99, metadata !37), !dbg !101
  %2 = bitcast %struct.G* %0 to i32 (...)***, !dbg !102
  %3 = getelementptr inbounds { [3 x i8*] }, { [3 x i8*] }* @_ZTV1G, i32 0, i32 0, i32 3, !dbg !102
  %4 = bitcast i8** %3 to i32 (...)**, !dbg !102
  store i32 (...)** %4, i32 (...)*** %2, align 8, !dbg !102
  ret void, !dbg !102
}
; CHECK: define void @_ZN1GC1Ev({0: si32 (...)**, 8: si8*, 16: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   si32 (...)*** %2 = bitcast %1
; CHECK:   si8** %3 = ptrshift @_ZTV1G, 24 * 0, 1 * 0, 8 * 3
; CHECK:   si32 (...)** %4 = bitcast %3
; CHECK:   store %2, %4, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1GC2Ev(%struct.G*, i8**) unnamed_addr #2 align 2 !dbg !125 {
  call void @llvm.dbg.value(metadata %struct.G* %0, i64 0, metadata !126, metadata !37), !dbg !127
  call void @llvm.dbg.value(metadata i8** %1, i64 0, metadata !128, metadata !37), !dbg !127
  %3 = load i8*, i8** %1, align 8, !dbg !129
  %4 = bitcast %struct.G* %0 to i32 (...)***, !dbg !129
  %5 = bitcast i8* %3 to i32 (...)**, !dbg !129
  store i32 (...)** %5, i32 (...)*** %4, align 8, !dbg !129
  ret void, !dbg !129
}
; CHECK: define void @_ZN1GC2Ev({0: si32 (...)**, 8: si8*, 16: {0: si32}}* %1, si8** %2) {
; CHECK: #1 !entry !exit {
; CHECK:   si8* %3 = load %2, align 8
; CHECK:   si32 (...)*** %4 = bitcast %1
; CHECK:   si32 (...)** %5 = bitcast %3
; CHECK:   store %4, %5, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline nounwind ssp uwtable
define linkonce_odr void @_ZN1HC1Ev(%struct.H*) unnamed_addr #2 align 2 !dbg !103 {
  call void @llvm.dbg.value(metadata %struct.H* %0, i64 0, metadata !108, metadata !37), !dbg !110
  %2 = bitcast %struct.H* %0 to %struct.F*, !dbg !111
  %3 = getelementptr inbounds [4 x i8*], [4 x i8*]* @_ZTT1H, i64 0, i64 1, !dbg !111
  call void @_ZN1FC2Ev(%struct.F* %2, i8** %3) #3, !dbg !111
  %4 = bitcast %struct.H* %0 to i8*, !dbg !111
  %5 = getelementptr inbounds i8, i8* %4, i64 16, !dbg !111
  %6 = bitcast i8* %5 to %struct.G*, !dbg !111
  %7 = getelementptr inbounds [4 x i8*], [4 x i8*]* @_ZTT1H, i64 0, i64 2, !dbg !112
  call void @_ZN1GC2Ev(%struct.G* %6, i8** %7) #3, !dbg !112
  %8 = bitcast %struct.H* %0 to i32 (...)***, !dbg !111
  %9 = getelementptr inbounds { [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i32 0, i32 0, i32 3, !dbg !111
  %10 = bitcast i8** %9 to i32 (...)**, !dbg !111
  store i32 (...)** %10, i32 (...)*** %8, align 8, !dbg !111
  %11 = bitcast %struct.H* %0 to i8*, !dbg !111
  %12 = getelementptr inbounds i8, i8* %11, i64 16, !dbg !111
  %13 = bitcast i8* %12 to i32 (...)***, !dbg !111
  %14 = getelementptr inbounds { [3 x i8*], [3 x i8*] }, { [3 x i8*], [3 x i8*] }* @_ZTV1H, i32 0, i32 1, i32 3, !dbg !111
  %15 = bitcast i8** %14 to i32 (...)**, !dbg !111
  store i32 (...)** %15, i32 (...)*** %13, align 8, !dbg !111
  ret void, !dbg !111
}
; CHECK: define void @_ZN1HC1Ev({0: <{0: si32 (...)**, 8: si32}>, 16: {0: si32 (...)**, 8: si8*}, 32: {0: si32}}* %1) {
; CHECK: #1 !entry !exit {
; CHECK:   <{0: si32 (...)**, 8: si32, 12: {0: si32}}>* %2 = bitcast %1
; CHECK:   si8** %3 = ptrshift @_ZTT1H, 32 * 0, 8 * 1
; CHECK:   call @_ZN1FC2Ev(%2, %3)
; CHECK:   si8* %4 = bitcast %1
; CHECK:   si8* %5 = ptrshift %4, 1 * 16
; CHECK:   {0: si32 (...)**, 8: si8*, 16: {0: si32}}* %6 = bitcast %5
; CHECK:   si8** %7 = ptrshift @_ZTT1H, 32 * 0, 8 * 2
; CHECK:   call @_ZN1GC2Ev(%6, %7)
; CHECK:   si32 (...)*** %8 = bitcast %1
; CHECK:   si8** %9 = ptrshift @_ZTV1H, 48 * 0, 1 * 0, 8 * 3
; CHECK:   si32 (...)** %10 = bitcast %9
; CHECK:   store %8, %10, align 8
; CHECK:   si8* %11 = bitcast %1
; CHECK:   si8* %12 = ptrshift %11, 1 * 16
; CHECK:   si32 (...)*** %13 = bitcast %12
; CHECK:   si8** %14 = ptrshift @_ZTV1H, 48 * 0, 1 * 24, 8 * 3
; CHECK:   si32 (...)** %15 = bitcast %14
; CHECK:   store %13, %15, align 8
; CHECK:   return
; CHECK: }
; CHECK: }

; Function Attrs: noinline norecurse nounwind ssp uwtable
define i32 @main() #0 !dbg !7 {
  %1 = alloca %struct.E, align 8
  %2 = alloca %struct.D, align 8
  %3 = alloca %struct.F, align 8
  %4 = alloca %struct.G, align 8
  %5 = alloca %struct.H, align 8
  call void @llvm.dbg.declare(metadata %struct.E* %1, metadata !11, metadata !37), !dbg !38
  call void @_ZN1EC1Ev(%struct.E* %1) #3, !dbg !38
  call void @llvm.dbg.declare(metadata %struct.D* %2, metadata !39, metadata !37), !dbg !48
  call void @_ZN1DC1Ev(%struct.D* %2) #3, !dbg !48
  call void @llvm.dbg.declare(metadata %struct.F* %3, metadata !49, metadata !37), !dbg !55
  call void @_ZN1FC1Ev(%struct.F* %3) #3, !dbg !55
  call void @llvm.dbg.declare(metadata %struct.G* %4, metadata !56, metadata !37), !dbg !62
  call void @_ZN1GC1Ev(%struct.G* %4) #3, !dbg !62
  call void @llvm.dbg.declare(metadata %struct.H* %5, metadata !63, metadata !37), !dbg !68
  call void @_ZN1HC1Ev(%struct.H* %5) #3, !dbg !68
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
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone
declare void @llvm.dbg.value(metadata, i64, metadata, metadata) #1

attributes #0 = { noinline norecurse nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 4.0.1 (tags/RELEASE_401/final)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "virtual-inheritance.cpp", directory: "/Users/marthaud/ikos/ikos-git/frontend/llvm/test/regression/import/basic_optimization")
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
!37 = !DIExpression()
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
!74 = !DILocation(line: 0, scope: !70)
!75 = !DILocation(line: 13, column: 8, scope: !70)
!76 = distinct !DISubprogram(name: "D", linkageName: "_ZN1DC1Ev", scope: !40, file: !1, line: 19, type: !77, isLocal: false, isDefinition: true, scopeLine: 19, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !80, variables: !2)
!77 = !DISubroutineType(types: !78)
!78 = !{null, !79}
!79 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !40, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!80 = !DISubprogram(name: "D", scope: !40, type: !77, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!81 = !DILocalVariable(name: "this", arg: 1, scope: !76, type: !82, flags: DIFlagArtificial | DIFlagObjectPointer)
!82 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !40, size: 64)
!83 = !DILocation(line: 0, scope: !76)
!84 = !DILocation(line: 19, column: 8, scope: !76)
!85 = distinct !DISubprogram(name: "F", linkageName: "_ZN1FC1Ev", scope: !50, file: !1, line: 23, type: !86, isLocal: false, isDefinition: true, scopeLine: 23, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !89, variables: !2)
!86 = !DISubroutineType(types: !87)
!87 = !{null, !88}
!88 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !50, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!89 = !DISubprogram(name: "F", scope: !50, type: !86, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!90 = !DILocalVariable(name: "this", arg: 1, scope: !85, type: !91, flags: DIFlagArtificial | DIFlagObjectPointer)
!91 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !50, size: 64)
!92 = !DILocation(line: 0, scope: !85)
!93 = !DILocation(line: 23, column: 8, scope: !85)
!94 = distinct !DISubprogram(name: "G", linkageName: "_ZN1GC1Ev", scope: !57, file: !1, line: 27, type: !95, isLocal: false, isDefinition: true, scopeLine: 27, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !98, variables: !2)
!95 = !DISubroutineType(types: !96)
!96 = !{null, !97}
!97 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !57, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!98 = !DISubprogram(name: "G", scope: !57, type: !95, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!99 = !DILocalVariable(name: "this", arg: 1, scope: !94, type: !100, flags: DIFlagArtificial | DIFlagObjectPointer)
!100 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !57, size: 64)
!101 = !DILocation(line: 0, scope: !94)
!102 = !DILocation(line: 27, column: 8, scope: !94)
!103 = distinct !DISubprogram(name: "H", linkageName: "_ZN1HC1Ev", scope: !64, file: !1, line: 31, type: !104, isLocal: false, isDefinition: true, scopeLine: 31, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !107, variables: !2)
!104 = !DISubroutineType(types: !105)
!105 = !{null, !106}
!106 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !64, size: 64, flags: DIFlagArtificial | DIFlagObjectPointer)
!107 = !DISubprogram(name: "H", scope: !64, type: !104, isLocal: false, isDefinition: false, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false)
!108 = !DILocalVariable(name: "this", arg: 1, scope: !103, type: !109, flags: DIFlagArtificial | DIFlagObjectPointer)
!109 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !64, size: 64)
!110 = !DILocation(line: 0, scope: !103)
!111 = !DILocation(line: 31, column: 8, scope: !103)
!112 = !DILocation(line: 31, column: 8, scope: !113)
!113 = !DILexicalBlockFile(scope: !103, file: !1, discriminator: 1)
!114 = distinct !DISubprogram(name: "f", linkageName: "_ZN1E1fEv", scope: !12, file: !1, line: 16, type: !34, isLocal: false, isDefinition: true, scopeLine: 16, flags: DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !33, variables: !2)
!115 = !DILocalVariable(name: "this", arg: 1, scope: !114, type: !73, flags: DIFlagArtificial | DIFlagObjectPointer)
!116 = !DILocation(line: 0, scope: !114)
!117 = !DILocation(line: 16, column: 21, scope: !114)
!118 = distinct !DISubprogram(name: "F", linkageName: "_ZN1FC2Ev", scope: !50, file: !1, line: 23, type: !86, isLocal: false, isDefinition: true, scopeLine: 23, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !89, variables: !2)
!119 = !DILocalVariable(name: "this", arg: 1, scope: !118, type: !91, flags: DIFlagArtificial | DIFlagObjectPointer)
!120 = !DILocation(line: 0, scope: !118)
!121 = !DILocalVariable(name: "vtt", arg: 2, scope: !118, type: !122, flags: DIFlagArtificial)
!122 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !123, size: 64)
!123 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!124 = !DILocation(line: 23, column: 8, scope: !118)
!125 = distinct !DISubprogram(name: "G", linkageName: "_ZN1GC2Ev", scope: !57, file: !1, line: 27, type: !95, isLocal: false, isDefinition: true, scopeLine: 27, flags: DIFlagArtificial | DIFlagPrototyped, isOptimized: false, unit: !0, declaration: !98, variables: !2)
!126 = !DILocalVariable(name: "this", arg: 1, scope: !125, type: !100, flags: DIFlagArtificial | DIFlagObjectPointer)
!127 = !DILocation(line: 0, scope: !125)
!128 = !DILocalVariable(name: "vtt", arg: 2, scope: !125, type: !122, flags: DIFlagArtificial)
!129 = !DILocation(line: 27, column: 8, scope: !125)
