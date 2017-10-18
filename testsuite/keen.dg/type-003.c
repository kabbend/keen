/* { dg-do compile } */
/* { dg-options "-w" } */


struct { int A[2]; } S;

int
main()
{
  S.A[0] = 1;	
  const int i = 3;
  i = 2; 	// { dg-error "" }
}	





