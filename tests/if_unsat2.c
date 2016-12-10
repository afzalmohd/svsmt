int main()
{
  int x, y, z;

  __CPROVER_assume((x < 10 && y > 10));

  if (x < 5) {
    z = x;
  } else {
    z = y;
  }

  z++;
  
  assert((z < 6 || z > 11));
  
  return 0;
}
