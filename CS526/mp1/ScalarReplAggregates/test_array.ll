; ModuleID = 'test_array.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

%struct.ST = type { i32, i32 }

; Function Attrs: nounwind uwtable
define void @foo(i32 %x) #0 {
  %1 = alloca i32, align 4
  store i32 %x, i32* %1, align 4
  %2 = load i32, i32* %1, align 4
  %3 = add nsw i32 %2, 1
  store i32 %3, i32* %1, align 4
  ret void
}

; Function Attrs: nounwind uwtable
define void @bar() #0 {
  %x = alloca [5 x i32], align 16
  %st = alloca [3 x %struct.ST], align 16
  %1 = getelementptr inbounds [5 x i32], [5 x i32]* %x, i64 0, i64 0
  store i32 0, i32* %1, align 16
  %2 = getelementptr inbounds [5 x i32], [5 x i32]* %x, i64 0, i64 1
  store i32 1, i32* %2, align 4
  %3 = getelementptr inbounds [5 x i32], [5 x i32]* %x, i64 0, i64 2
  store i32 2, i32* %3, align 8
  %4 = getelementptr inbounds [5 x i32], [5 x i32]* %x, i64 0, i64 3
  store i32 3, i32* %4, align 4
  %5 = getelementptr inbounds [5 x i32], [5 x i32]* %x, i64 0, i64 4
  store i32 4, i32* %5, align 16
  %6 = getelementptr inbounds [5 x i32], [5 x i32]* %x, i64 0, i64 0
  %7 = load i32, i32* %6, align 16
  call void @foo(i32 %7)
  %8 = getelementptr inbounds [5 x i32], [5 x i32]* %x, i64 0, i64 1
  %9 = load i32, i32* %8, align 4
  call void @foo(i32 %9)
  %10 = getelementptr inbounds [5 x i32], [5 x i32]* %x, i64 0, i64 2
  %11 = load i32, i32* %10, align 8
  call void @foo(i32 %11)
  %12 = getelementptr inbounds [3 x %struct.ST], [3 x %struct.ST]* %st, i64 0, i64 1
  %13 = getelementptr inbounds %struct.ST, %struct.ST* %12, i32 0, i32 0
  store i32 1, i32* %13, align 8
  %14 = getelementptr inbounds [3 x %struct.ST], [3 x %struct.ST]* %st, i64 0, i64 1
  %15 = getelementptr inbounds %struct.ST, %struct.ST* %14, i32 0, i32 1
  store i32 2, i32* %15, align 4
  %16 = getelementptr inbounds [3 x %struct.ST], [3 x %struct.ST]* %st, i64 0, i64 1
  %17 = getelementptr inbounds %struct.ST, %struct.ST* %16, i32 0, i32 0
  %18 = load i32, i32* %17, align 8
  call void @foo(i32 %18)
  ret void
}

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0-2ubuntu4 (tags/RELEASE_380/final)"}
