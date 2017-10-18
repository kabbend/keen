/* { dg-do compile } */

// forward declarations
int printf(char *, ...);
void abort();

int
main(int argc, char **argv)
{
  int x = sizeof (int);
  printf("sizeof(int)=%d\n",x);
  if (x != 4) abort();
  x = sizeof (char);
  printf("sizeof(char)=%d\n",x);
  if (x != 1) abort();
  x = sizeof (unsigned char);
  printf("sizeof(unsigned char)=%d\n",x);
  if (x != 1) abort();
  x = sizeof (signed char);
  printf("sizeof(signed char)=%d\n",x);
  if (x != 1) abort();
  x = sizeof (int [10]);
  printf("sizeof(int [10])=%d\n",x);
  if (x != 40) abort();
  x = sizeof (int *[10]);
  printf("sizeof(int *[10])=%d\n",x);
  if (x != 40) abort();
  x = sizeof (int [][10]);		// { dg-error "incomplete type" }
  printf("sizeof(int [][10])=%d\n",x);
  if (x != 40) abort();

}


