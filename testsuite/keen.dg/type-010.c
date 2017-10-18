/* { dg-do compile } */
/* { dg-options "-w" } */


int
main()
{
  struct { const int x; } S1;
  struct { int x; } S2;
  S1 = S2 ;  // { dg-error "" } not allowed
}	





