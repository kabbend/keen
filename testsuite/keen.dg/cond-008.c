/* { dg-do "compile" } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();

int
main()
{

  int A = 2;
  int B[5];
  int y;

  int x = (y < 2)? A:B;   // { dg-error "invalid operands" }

}


