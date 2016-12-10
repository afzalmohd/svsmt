void main ()
 {
  int x,y; 
  __CPROVER_assume(y<2 && x < 1); 
 // x=x+2;
  if(x<2)
  {
   // y=y+5;
  	if(y>3)
  	{
  		y=y-1;
  	}
  	else
  	{
  		x=x+1;
  	}
   
  }
  else
  {
    x=x-3;
   
  }
 // y=y-x; 
  assert(x < 4);
}
