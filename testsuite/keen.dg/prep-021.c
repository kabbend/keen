/* { dg-do "run" } */
/* { dg-options "-w" } */


#define NULL	0

#define A(x)	( x == NULL && x == NULL && x == NULL)

int
main()
{
  if A(1) abort(); // check that several instances of NULL are substituted. If not the case, a compilation
		   // error on unknown NULL will appear 
  return 0; 
}

