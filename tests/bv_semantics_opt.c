int main()
{
  int x, y;

  __CPROVER_assume((x < 10 && y > 5));

  if (x < 2) {
    y = x + y;
    x = x + 2;
  } else {
    y = x + y;
    x = x + 2;
  }

  assert(y != 0);
  
  return 0;
}
