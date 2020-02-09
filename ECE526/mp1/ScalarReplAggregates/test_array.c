struct ST {
  int x;
  int y;
};

void foo(int x) {
  x = x+1;
}

void bar(void) {
  int x[5];
  x[0] = 0;
  x[1] = 1;
  x[2] = 2;
  x[3] = 3;
  x[4] = 4;
  foo(x[0]);
  foo(x[1]);
  foo(x[2]);

  struct ST st[3];
  st[1].x = 1;
  st[1].y = 2;
  foo(st[1].x);
}
