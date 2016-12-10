int main()
{
  int x, y;

  __CPROVER_assume(x < 10);

  if (x < 5) {
    y = x + x;
  } else {
    y = x;
  }
  
  y++;

  if (y < 10) {
    x = 0;
  } else {
    x = 10;
  }
  
  assert(x != 0);
  
  return 0;
}
