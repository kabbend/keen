/* { dg-do compile } */
/* { dg-options "-w" } */


struct { const int A[2]; } S;

int
main()
{
  S.A[0] = 1;		/* { dg-error "" } */
  const int i = 0;
  i = 1;
}	





