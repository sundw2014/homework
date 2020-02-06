%struct.RT = type { i8, i8, i8 }
%struct.ST = type { i32, double, %struct.RT }

define i32 @foo() nounwind uwtable readnone optsize ssp {
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
