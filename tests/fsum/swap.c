int x, y;

void swap()
{
  int t;
  t = x;
  x = y;
  y = t;
}

void swap_plus1()
{
  int t;
  t = x;
  x = y+1;
  y = t;
}

void bar()
{
  assert(x < 3 && y > 2);
}

int main()
{
  __CPROVER_assume(x > 2 && y < 3);
  swap();
  bar();
  //  assert(x < 3 && y > 2);
}
