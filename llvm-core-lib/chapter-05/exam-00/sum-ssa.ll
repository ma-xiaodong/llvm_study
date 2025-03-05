; ModuleID = 'sum-ssa.bc'
source_filename = "sum.ll"
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

define i32 @sum(i32 %a, i32 %b) {
entry:
  %add = add i32 %a, %b
  ret i32 %add
}
