/*
 * function call, recursive call within function call
 */

/* { dg-do "run" } */
/* { dg-options "-w" } */

int
factorielle (int x)
{
  if (x == 1) return 1;
  return x * factorielle( x - 1 );
}

int
main ()
{ 
 int x = 3;
 printf("now trying to compute fact(3!). (3!)! = %d\n", factorielle(factorielle(x)));
 x = factorielle(factorielle(x));
 if (x != 720) abort();
 return 0;
}

