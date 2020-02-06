%struct.RT = type { i8, i8, i8 }
%struct.ST = type { i32, double, %struct.RT }
%struct.ST_1 = type { i32, double, %struct.RT* }

define i32 @test1() nounwind uwtable readnone optsize ssp {
entry:
  %p0 = alloca %struct.ST
  %p1 = getelementptr inbounds %struct.ST, %struct.ST* %p0, i64 0, i32 0
  %p2 = getelementptr inbounds %struct.ST, %struct.ST* %p0, i64 0, i32 1
  %p3 = getelementptr inbounds %struct.ST, %struct.ST* %p0, i64 0, i32 2
  %p4 = getelementptr inbounds %struct.ST, %struct.ST* %p0, i64 0, i32 2, i32 1
  %p4_1 = getelementptr inbounds %struct.RT, %struct.RT* %p3, i64 0, i32 1
  store i32 1, i32* %p1
  store double 2., double* %p2
  store i8 3, i8* %p4
  store i8 4, i8* %p4_1
  ret i32 0
}

define i32 @test2() nounwind uwtable readnone optsize ssp {
entry:
  %0 = alloca i32
  store i32 0, i32* %0
  %1 = alloca %struct.RT*
  %p = alloca %struct.RT
  store %struct.RT* %p, %struct.RT** %1
  %2 = load %struct.RT*, %struct.RT** %1
  %3 = getelementptr inbounds %struct.RT, %struct.RT* %2, i64 0, i32 1
  store i8 0, i8* %3
  ret i32 0
}

define i32 @test3() nounwind uwtable readnone optsize ssp {
entry:
  %0 = alloca %struct.ST_1
  %1 = getelementptr inbounds %struct.ST_1, %struct.ST_1* %0, i64 0, i32 2
  %p = alloca %struct.RT
  store %struct.RT* %p, %struct.RT** %1
  %2 = load %struct.RT*, %struct.RT** %1
  %3 = getelementptr inbounds %struct.RT, %struct.RT* %2, i64 0, i32 1
  store i8 0, i8* %3
  ret i32 0
}
