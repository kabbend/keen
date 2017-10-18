/* { dg-do "run" } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();

#define A_MACRO

#if 0
  #  undef A_MACRO
#endif

int f()
{
#if defined A_MACRO 
 return 0;
#elif 1 == 2
 return 1;
#else 
 return 2;
#endif
}

int
main()
{
  if (f() != 0) abort();
  return 0;
}

