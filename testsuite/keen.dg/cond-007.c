/* { dg-do "compile" } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();

int
main()
{
  int y = 2;
  const z = 2;
  int *P1 = &y;
  int *P2 = &z;

  int x = (y < 2)? P1:P2;  // should not raise an error

}


