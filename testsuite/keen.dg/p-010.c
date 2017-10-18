
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main ()
{
  int x = 3;
  int *y = &x;
  int **z = &y;
  printf("**z=%d\n",**z);
  if (**z != 3) abort();
  return 0;
}

