int x, y, z;

void foo()
{
  x = x + 1;
}

void bar()
{
  y = y + 1;
}

int main()
{
  __CPROVER_assume(x == 4 && y == 6);

  if (z > 0) {
    foo();
  } else {
    bar();
  }

  assert(x != 4 || y != 6);
}
