int main()
{
  unsigned int x, y;

  __CPROVER_assume((x < 10 && y > 5));

  x = x + y;
  y = y + x;
  x = x + 2*x;
  y = y + 2*y;

  //x = 10 and y = 5 satisfies this
  assert((x == 45 && y == 60));
  return 0;
}
