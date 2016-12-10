int main()
{
  int x, y;

  __CPROVER_assume((x < 10 && y > 5));

  x = x + y;
  y = y + x;
  x = x + 2*x;
  y = y + 2*y;

  //x = 0 and y = 6 satisfies this
  assert((x == 18 && y == 36));
  return 0;
}
