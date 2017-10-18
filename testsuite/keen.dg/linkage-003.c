/* { dg-do "compile" } */
/* { dg-options "-w" } */

extern int A;

int
main()
{
 
  int x = A;
  printf("x=A=%d\n",x);
  if (x != 2) abort();
  return 0;

}

