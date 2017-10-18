/* { dg-do compile } */
/* { dg-options "-w" } */


int
main()
{
  typedef struct { int x; } S;
  const S S1;
  struct { int x; } S2;
  S1 = S2 ;  // { dg-error "" } not allowed
}	





