/* { dg-do compile } */
/* { dg-options "-w" } */


const int A[2][3][5];

int
main()
{
  A[0][0][0] =  1 ; // { dg-error "" }
}	





