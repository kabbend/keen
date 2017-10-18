/* { dg-do "compile" } */
/* { dg-options "-w" } */


#define NULL	0

#define A(x)	( A(x) && x == NULL && x == NULL && x == NULL)

int
main()
{
  if A(1) return 1; // check we do not enter infinite loop
}

