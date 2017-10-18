/* { dg-do "run" } */

// forward declarations
int printf(char *, ...);
void abort();

#define NOTNULL 99

int f(int x)
{
 printf("received x=%d\n",x);
 if (x == -1) abort();
}

int
main()
{
  int x = ( 2 > 3 )? 1 : 3;
  printf("x=%d\n",x);
  if (x!=3) abort();

  x = ( x > 2 )? 1 : 3;
  printf("x=%d\n",x);
  if (x!=1) abort();

  f( (NOTNULL == 0)? -1:+1);
  return 0;

}


