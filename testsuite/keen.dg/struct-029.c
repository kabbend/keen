
/* { dg-do "run" } */
/* { dg-options "-w" } */

struct a { int x; int y; int c[3]; };

void
f (struct a P)
{
 printf("P.x=%d\n",P.x);
 printf("P.y=%d\n",P.y);
 P.x++;
 P.c[2] = 12;
}

int
main ()
{
  struct a A_struct;
  A_struct.x = 8;
  A_struct.y = 9;
  A_struct.c[2] = 22;
  f(A_struct);
  // check that call to f() does not alter origin struct A_struct
  printf("A_struct.x=%d\n",A_struct.x);
  if (A_struct.x != 8) abort();
  printf("A_struct.c[2]=%d\n",A_struct.c[2]);
  if (A_struct.c[2] != 22) abort();

}

