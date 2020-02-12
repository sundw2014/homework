// RUN: clang -O0 -S -emit-llvm %s -o - | opt -load /home/daweis2/llvm-8.0.1.src/build/lib/LLVMSRAOdaweis2.so -inline -globaldce -instcombine -argpromotion -sccp -dce -simplifycfg -globaldce -scalarrepl-daweis2 -mem2reg -verify -S 2<&1 | FileCheck %s

// CHECK: NUM_OUTER_ITERATIONS 7

struct ST {
  int x;
  struct ST* p;
};

void foo(int x) {
}

int test(void) {
  struct ST st1;
  struct ST st2;
  struct ST st3;
  struct ST st4;
  struct ST st5;

  st1.p = &st2;
  st2.p = &st3;
  st3.p = &st4;
  st4.p = &st5;

  foo(st1.x);
  foo(st2.x);
  foo(st3.x);
  foo(st4.x);
  foo(st5.x);
// CHECK: @test
// CHECK-NOT: alloca
  return st1.p->p->p->p->x;
}
