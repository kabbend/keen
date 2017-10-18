/*
 * function call, recursive call, assignment within function call
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
 int ret; 
 printf("now trying to compute 10!. 10! = %d\n", ret = factorielle(10));
 if (ret != 3628800 ) abort();
 return 0;
}

