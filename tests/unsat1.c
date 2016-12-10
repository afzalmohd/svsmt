int main()
{
  unsigned int x, y;

  __CPROVER_assume((x < 10 && y < 10));

  x = x * y; // < 100
  y = y * x; // < 1000
  x = x + y; // < 1100
  y = 2*x;  // < 2200

  //x = 0 and y = 6 satisfies this
  assert(y < 2200);
  return 0;
}
