int main()
{
  unsigned int x, y, z;

  __CPROVER_assume((x < 10 && y > 5));

  if (x == 5) {
    z = x + y;
  } else {
    z = y - x;
  }

  assert(z > 0);
  
  return 0;
}
