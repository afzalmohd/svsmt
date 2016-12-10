//Example from the paper "Compositional Safety Refutation Techniques"
int x, y, z;

void foo()
{

  x = x + 1;
}

void bar()
{
  assert(x > 10);
}

int main()
{
  //  __CPROVER_assume((x < 128 && y > 256));

  //  _CPROVER_assume(x > 0);
  if (x < 0) {
    foo();
    foo();
    bar();
  }
  
  return 0;
}
