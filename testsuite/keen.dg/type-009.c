/* { dg-do compile } */
/* { dg-options "-w" } */


int
main()
{
  int * const p;
  int x = 1;
  p = &x;  // { dg-error "" } not allowed
}	





