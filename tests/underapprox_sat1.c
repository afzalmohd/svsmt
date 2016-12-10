int main()
{
  int x, y;

  __CPROVER_assume((x < 128 && y > 256));

  x = x + y;
  y = y + x;
  x = x + 2*x;
  y = y + 2*y;


  assert((x == 224 && y == 331));
  return 0;
}
