/* { dg-do "run" } */
/* { dg-options "-w" } */

extern int A;

int A = 2;

int
main()
{
 
  extern int A;
  int x = A;
  printf("x=A=%d\n",x);
  if (x != 2) abort();
  return 0;

}

