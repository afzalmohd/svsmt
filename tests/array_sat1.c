int main()
{
  int x, y;
  int array[10];

  __CPROVER_assume((x < 10 && y < 10));

  array[x] = y;
  array[y] = x;
    
  if (x == 5) {
    y = y - 2*x;
  } else {
    y = y - x;
  }

  assert(y != 0);
  
  return 0;
}
