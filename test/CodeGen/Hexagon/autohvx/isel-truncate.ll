; RUN: llc -march=hexagon < %s | FileCheck %s

; Check that this compiles successfully.
; CHECK: vpacke

target datalayout = "e-m:e-p:32:32:32-a:0-n16:32-i64:64:64-i32:32:32-i16:16:16-i1:8:8-f32:32:32-f64:64:64-v32:32:32-v64:64:64-v512:512:512-v1024:1024:1024-v2048:2048:2048"
target triple = "hexagon"

; Function Attrs: norecurse nounwind
define void @fred() #0 {
b0:
  %v1 = select <16 x i1> undef, <16 x i32> undef, <16 x i32> zeroinitializer
  %v2 = trunc <16 x i32> %v1 to <16 x i16>
  store <16 x i16> %v2, <16 x i16>* undef, align 2
  ret void
}

attributes #0 = { norecurse nounwind "target-cpu"="hexagonv60" "target-features"="+hvx-length64b,+hvxv60" }
