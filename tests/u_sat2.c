int main()
{
  int x, y;

  __CPROVER_assume((x < 128));

  x = x + y;
  y = y + x;
  x = x + 2*x;
  y = y + 2*y;


  assert((x < 1024));
  return 0;
}
