/* { dg-do "run" } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();

#define A_MACRO
#undef A_MACRO

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
  if (f() != 2) abort();
  return 0;
}

