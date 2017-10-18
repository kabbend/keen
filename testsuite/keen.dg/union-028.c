
/* { dg-do "run" } */
/* { dg-options "-w" } */

union a { int x; int y; int c[3]; };

void
f (union a P)
{
 P.c[2] = 12;
}

int
main ()
{
  union a A_union;
  A_union.c[2] = 22;
  f(A_union);
  // check that call to f() does not alter origin union A_union
  printf("A_union.c[2]=%d\n",A_union.c[2]);
  if (A_union.c[2] != 22) abort();

}

