int x, y, z;

void foo()
{

  int t;
  t = x;
  x = t + 2;
  y = y + x;
}

int main()
{
  __CPROVER_assume(x == z);

  foo();
  foo();
  foo();
  
  y = y + 2;
  
  x = x + 3;

  assert(x > z);

  return 0;
}
