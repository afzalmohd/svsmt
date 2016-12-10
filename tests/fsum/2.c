int x, y, z;

void foo()
{
  if (y < 0) {
    x = y + y;
  } else {
    y = x + x;
  }
  x = x + 1;
}

void bar()
{
  assert((x + y) == 0);
}


int main()
{
  //  __CPROVER_assume((x < 128 && y > 256));

  __CPROVER_assume(y == (x + 2));
  if ( x < 0) {
    foo();
    foo();
    bar();
  }
 
  return 0;
}
