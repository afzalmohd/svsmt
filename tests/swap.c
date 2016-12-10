int main()
{
  int x, y, z;

  __CPROVER_assume(((x == 6) && (y == 5)));

  z = x;
  x = y;
  y = z;

  assert((x == 5 && y == 6));
  
  return 0;
}
