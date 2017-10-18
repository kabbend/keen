/* { dg-do "compile" } */

// forward declarations
int printf(char *, ...);
void abort();

int
main()
{
  int y = 2;
   
  int *pnotnull = &y;
  int *pnull = (int *) 0;

  int x = (y < 2)? pnotnull:pnull; // { dg-warning "without a cast" }

  printf("x=%d\n",x);

  if (x != 0) abort(); 
}


