int main()
{
  int x, y;

  __CPROVER_assume((x < 10 && y > 5));

  if (x == 5) {
    y = y - 2*x;
  } else {
    y = y - x;
  }

  assert(y != 0);
  
  return 0;
}
