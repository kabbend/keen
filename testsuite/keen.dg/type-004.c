/* { dg-do compile } */
/* { dg-options "-w" } */


int
main()
{
  typedef int A;
  const A i = 3;
  i = 2; 	// { dg-error "" }
}	





