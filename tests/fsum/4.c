int x, y, z;

void foo()
{

  __CPROVER_assume(x !=	1 && y != 0);
  z = z*2;
  y = x + y;
  if (y > 10) {
    //foo();
    x = 9;
  } else {
    assert(y == 2);
  }

}

int main()
{
  //  __CPROVER_assume((x < 128 && y > 256));

  foo();
  
  y = y + 2;
  x = x + 3;

  assert(x + y > 10);

  //wp is 
  return 0;
}
