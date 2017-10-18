/* { dg-do "compile" } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();

int
main()
{
  int y = 2;
  struct a { int x; }; 
  struct a *pnotnull = &y;

  int x = (y < 2)? pnotnull:(int *)0;  // { dg-error "invalid operands" } 

  printf("x=%d\n",x);

  if (x != 0) abort(); 
}


