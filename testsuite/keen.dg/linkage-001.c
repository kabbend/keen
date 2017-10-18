/* { dg-do "run" } */
/* { dg-options "-w" } */

extern int A;

int A = 2;

int
main()
{
 
  int x = A;
  printf("x=A=%d\n",x);
  if (x != 2) abort();
  return 0;

}

