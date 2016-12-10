int main()
{
  int x, y;
  __CPROVER_assume(x < 10);

  if (x == 3) {
    y = 1;
  } else {
    y = 1;
  }
  if (y == 10) {
    x = 4*y;
  } else {
    x = 20;
  }
  /* if (x < 10) { */
  /*   y = x * x; */
  /*   y = y * x; */
  /* }else {  */
  /*   x = x + 3;  */
  /* } */
  /* y = 10; */
  /* x = x + y; */
  /* y = x - y; */
  /* x = x + 2*y; */

  assert(x == 20);
  
  return 0;
}
