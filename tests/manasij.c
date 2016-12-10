int main()
{
  int x, y, z;

  __CPROVER_assume(z == 6);

  if (x >= 0) {
    y = z;
  } else {
    y = z;
  }

  assert(y > 0);
  
  return 0;
}
