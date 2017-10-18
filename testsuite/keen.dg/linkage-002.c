/* { dg-do "run" } */
/* { dg-options "-w" } */

int A = 2;

extern int A;

int
main()
{
 
  int x = A;
  printf("x=A=%d\n",x);
  if (x != 2) abort();
  return 0;

}

