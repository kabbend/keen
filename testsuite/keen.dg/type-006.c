/* { dg-do compile } */
/* { dg-options "-w" } */


int
main()
{
  typedef const const int A; // { dg-error "duplicate" } this should not be allowed
  const A x = 1; 
  if (x != 1) abort(); 
  return 0;
}	





