/* { dg-do "compile" } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();

#define A_MACRO

#if defined A_MACRO 		
# undef A_MACRO
#endif

int f()
{
 return 0;
#else  		// { dg-error "" }
 return 2;
#endif
}

int
main()
{
  if (f() != 2) abort();
  return 0;
}

