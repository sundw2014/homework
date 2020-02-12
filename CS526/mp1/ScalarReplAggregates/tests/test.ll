; RUN: opt < %s -load /home/daweis2/llvm-8.0.1.src/build/lib/LLVMSRAOdaweis2.so -inline -globaldce -argpromotion -sccp -dce -simplifycfg -globaldce -scalarrepl-daweis2 -mem2reg -verify -S | FileCheck %s

%struct.RT = type { i32, i32, i32 }
%struct.ST = type { i32, double, %struct.RT }
%struct.QT = type { i32, double, %struct.RT* }

define i32 @foo(i32 %x) {
  ret i32 %x
}

define i32 @bar(%struct.RT* %ptr) {
  %int = ptrtoint %struct.RT* %ptr to i32
  ret i32 %int
}

; alloca of a struct. struct in a struct
define i32 @test1() nounwind uwtable readnone optsize ssp {
entry:
  %p0 = alloca %struct.ST
  %p1 = getelementptr inbounds %struct.ST, %struct.ST* %p0, i64 0, i32 0 ; p1 = &(p0->e0)
  %p3 = getelementptr inbounds %struct.ST, %struct.ST* %p0, i64 0, i32 2 ; p3 = &(p0->e2)
  %p4 = getelementptr inbounds %struct.ST, %struct.ST* %p0, i64 0, i32 2, i32 1 ;p4 = &(p0->e2->e1)
  %p5 = getelementptr inbounds %struct.RT, %struct.RT* %p3, i64 0, i32 2 ; p5 = &(p3->e2) (== &(p0->e2->e2))
  store i32 1, i32* %p1 ; *p1 = 1
  store i32 4, i32* %p4 ; *p4 = 3
  store i32 5, i32* %p5 ; *p5 = 5

  %v1 = load i32, i32* %p1 ; %v1 = *p1
  %v4 = load i32, i32* %p4 ; %v4 = *p4
  %v5 = load i32, i32* %p5 ; %v5 = *p5

  %0 = call i32 @foo(i32 %v1)
  %1 = call i32 @foo(i32 %v4)
  %2 = call i32 @foo(i32 %v5)

  %3 = add i32 %v1, %v4
  %4 = add i32 %3, %v5

  ret i32 %4
; CHECK: @test1
; CHECK-NOT: alloca
; CHECK: ret

}

; icmp
define i32 @test2() {
  %p0 = alloca %struct.ST
  %p1 = getelementptr inbounds %struct.ST, %struct.ST* %p0, i64 0, i32 0 ; p1 = &(p0->e0)
  store i32 1, i32* %p1 ; *p1 = 1

  %v = load i32, i32* %p1 ; %v = *p1

  call i32 @foo(i32 %v)

  %v1 = icmp ne %struct.ST* %p0, null
  %v2 = icmp eq %struct.ST* %p0, null
  %v3 = icmp ne i32* %p1, null
  %v4 = icmp eq i32* %p1, null

  %vv1 = zext i1 %v1 to i32
  %vv2 = zext i1 %v2 to i32
  %vv3 = zext i1 %v3 to i32
  %vv4 = zext i1 %v4 to i32

  call i32 @foo(i32 %vv1)
  call i32 @foo(i32 %vv2)
  call i32 @foo(i32 %vv3)
  call i32 @foo(i32 %vv4)

  %t1 = add i32 %vv1, %vv2
  %t2 = add i32 %t1, %vv3
  %t3 = add i32 %t2, %vv4
  ret i32 %t3
; CHECK: @test2
; CHECK-NOT: alloca %struct.ST
; CHECK: alloca i32
; CHECK: %vv1 = zext i1 true to i32
; CHECK: %vv2 = zext i1 false to i32
; CHECK: ret

}

; address of a struct is used in store's operand 0. This needs two outer iteration.
define i32 @test3() nounwind uwtable readnone optsize ssp {
entry:
  %p_qt = alloca %struct.QT
  %pp_rt = getelementptr inbounds %struct.QT, %struct.QT* %p_qt, i64 0, i32 2 ; pp_rt = &(p_qt->e2)
  %p_rt = alloca %struct.RT
  store %struct.RT* %p_rt, %struct.RT** %pp_rt ; *pp_rt = p_rt
  %ptr = load %struct.RT*, %struct.RT** %pp_rt ; ptr = *pp_rt (== p_rt)
  %p_i32 = getelementptr inbounds %struct.RT, %struct.RT* %ptr, i64 0, i32 1 ; p_i32 = &(ptr->e1)
  store i32 3, i32* %p_i32 ; *p_i32 = 3

  %v_i32 = load i32, i32* %p_i32 ; v_i32 = *p_i32

  call i32 @foo(i32 %v_i32)
  ret i32 %v_i32

; CHECK: @test3
; CHECK-NOT: alloca
; CHECK: ret

}


; use of address. Do not touch it.
define i32 @test4() {
entry:
  %p_rt = alloca %struct.RT
  %p_i32 = getelementptr inbounds %struct.RT, %struct.RT* %p_rt, i64 0, i32 1 ; p_i32 = &(p_qt->e1)
  store i32 3, i32* %p_i32 ; *p_i32 = 3
  %v_i32 = load i32, i32* %p_i32 ; v_i32 = *p_i32

  call i32 @foo(i32 %v_i32)
  %int_ptr = call i32 @bar(%struct.RT* %p_rt)

  %t1 = add i32 %v_i32, %int_ptr
  ret i32 %t1

; CHECK: @test4
; CHECK: alloca %struct.RT
; CHECK: ret

}

; volatile
define i32 @test5() {
entry:
  %p_rt = alloca %struct.RT
  %p_i32 = getelementptr inbounds %struct.RT, %struct.RT* %p_rt, i64 0, i32 1 ; p_i32 = &(p_qt->e1)
  store i32 3, i32* %p_i32 ; *p_i32 = 3
  %v_i32 = load volatile i32, i32* %p_i32 ; v_i32 = *p_i32

  call i32 @foo(i32 %v_i32)
  ret i32 %v_i32

; CHECK: @test5
; CHECK: alloca %struct.RT
; CHECK: ret

}

; alloca of an array
define i32 @test6() #0 {
entry:
  %p_a_qts = alloca [3 x %struct.QT]
  %p_qt = getelementptr inbounds [3 x %struct.QT], [3 x %struct.QT]* %p_a_qts, i64 0, i64 1 ; p_qt = &(p_a_qts[1])
  %pp_rt = getelementptr inbounds %struct.QT, %struct.QT* %p_qt, i64 0, i32 2 ; pp_rt = &(p_qt->e2)
  %p_rt = alloca %struct.RT
  store %struct.RT* %p_rt, %struct.RT** %pp_rt ; *pp_rt = p_rt
  %ptr = load %struct.RT*, %struct.RT** %pp_rt ; ptr = *pp_rt (== p_rt)
  %p_i32 = getelementptr inbounds %struct.RT, %struct.RT* %ptr, i64 0, i32 1 ; p_i32 = &(ptr->e1)
  store i32 3, i32* %p_i32 ; *p_i32 = 3

  %v_i32 = load i32, i32* %p_i32 ; v_i32 = *p_i32

  call i32 @foo(i32 %v_i32)
  ret i32 %v_i32
; CHECK: test6
; CHECK-NOT: alloca
; CHECK: ret

}

; non-const index. Do not touch it.
define i32 @test7() #0 {
entry:
  %pid = alloca i32
  %id = load volatile i32, i32* %pid
  %p_a_qts = alloca [3 x %struct.QT]
  %p_qt = getelementptr inbounds [3 x %struct.QT], [3 x %struct.QT]* %p_a_qts, i64 0, i32 %id; p_qt = &(p_a_qts[id])
  %pp_rt = getelementptr inbounds %struct.QT, %struct.QT* %p_qt, i64 0, i32 2 ; pp_rt = &(p_qt->e2)
  %p_rt = alloca %struct.RT
  store %struct.RT* %p_rt, %struct.RT** %pp_rt ; *pp_rt = p_rt
  %ptr = load %struct.RT*, %struct.RT** %pp_rt ; ptr = *pp_rt (== p_rt)
  %p_i32 = getelementptr inbounds %struct.RT, %struct.RT* %ptr, i64 0, i32 1 ; p_i32 = &(ptr->e1)
  store i32 3, i32* %p_i32 ; *p_i32 = 3

  %v_i32 = load i32, i32* %p_i32 ; v_i32 = *p_i32

  call i32 @foo(i32 %v_i32)
  ret i32 %v_i32
; CHECK: @test7
; CHECK: alloca i32
; CHECK: alloca [3 x %struct.QT]
; CHECK: ret

}
