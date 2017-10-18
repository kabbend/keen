
/* { dg-do "run" } */
/* { dg-options "-w" } */

int f()
{
 abort();
}

int
evaluate_me(int x)
{
 printf("arg=%d\n",x);
 return x;
}

int 
main()
{

  int x = 1;

  // check that && or || 1st condition is evaluated first
  // and may not require additional evaluation
  int z = !x && f();
  int t = evaluate_me(1) || f() ;
  if ( z != 0 || t != 1 ) abort();
 
  return 0;
}

