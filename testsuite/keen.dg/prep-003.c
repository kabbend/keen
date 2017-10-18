/* { dg-do "compile" } */
/* { dg-options "-w" } */

// forward declarations
int printf(char *, ...);
void abort();

#if 	/* { dg-error "uncomplete" } */

int
main()
{
}

#endif

