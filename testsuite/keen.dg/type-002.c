/* { dg-do run } */
/* { dg-options "-w" } */


struct { int A[2]; } S;

int
main()
{
  S.A[0] = 1;	
  const int i = 3;
  printf ("i=%d\n",i);
  if (i != 3) abort();
  return 0;
}	





