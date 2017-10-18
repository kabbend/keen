/* { dg-do "run" } */

// forward declarations
int printf(char *, ...);
void abort();

int
main()
{
  int x = 1;
  int y = 2;
  x = (x > y)? x:(x==1)? 3:5; 
  printf("x=%d\n",x);
  return 0;
}


