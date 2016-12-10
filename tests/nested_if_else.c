void main ()
 {
  int x,y; 
  __CPROVER_assume(x < 2 && y < 1); 
 // x=x+2;
  if(x>2)
  {
   // y=y+5;
    if(y>3)
    {
      y=y+1;
    }
    else
    {
      if(y<2)
      {
        x=x+2;
      }
      else
      {
        
      }
      x=x+1;
    }
   
  }
  else
  {
    x=x-3;
   
  }
  if(y==0)
  {
  x=x+1;
  }
  else
  {
  y=y+1;
  }
 // y=y-x; 
  assert(y<2);
}
