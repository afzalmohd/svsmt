int main()
{
  int x,y;

  __CPROVER_assume(1 == 1);

  y = 5;
  x = y;
  
  if (x < 5 || x > 6) {
    x = x*2;
    if (y > 10 && y < 20) {
      y++;
    } else {
      x--;
    }
    x++;
  }    

  assert(x <= 10);
  
  return 0;
}
