/* { dg-do "run" } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();

int f()
{
#if 1 == 0 
 return 0;
#elif 1 == 1
 return 1;
#else 
 return 2;
#endif
}

int
main()
{
  if (f() != 1) abort();
  return 0;
}

