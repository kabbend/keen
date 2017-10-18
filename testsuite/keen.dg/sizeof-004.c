/* { dg-do "run" } */

// forward declarations
int printf(char *, ...);
void abort();

void f( int A[3] )
{
  int x = sizeof A;
  printf("sizeof x (passed as parameter) = %d\n", x);
  if (x != 4) abort();
}

void g( int *A )
{
  int x = sizeof A;
  printf("sizeof x (passed as parameter) = %d\n", x);
  if (x != 4) abort();
}

int
main(int argc, char **argv)
{
  int ARRAY[3] = { 0, 1, 2 };
  int x = sizeof ARRAY;
  printf("sizeof ARRAY = %d\n", x);
  if (x != 12) abort();
  printf("ARRAY[2]=%d\n", ARRAY[2]);
  x = sizeof ARRAY[2]++;
  printf("sizeof ARRAY[2]++ = %d\n", x);
  printf("ARRAY[2]=%d\n", ARRAY[2]);
  if (x != 4) abort();
  if (ARRAY[2]!=2) abort();
  f(ARRAY);
  g(ARRAY);
}


