/* { dg-do compile } */
/* { dg-options "-w" } */


int
main()
{
  const int *p;
  int x = 1;
  *p = 2; // { dg-error "" } not allowed
}	





