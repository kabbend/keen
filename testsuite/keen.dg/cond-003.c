/* { dg-do "compile" } */

// forward declarations
int printf(char *, ...);
void abort();

int
main()
{
  int x = 1;
  int y = 2;
  (x > 2)? x:y = 3; // { dg-error "not allowed here" }
  return 0;
}


