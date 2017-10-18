/* { dg-do run } */
/* { dg-options "-w" } */


int
main()
{
  typedef const int A;
  const A x = 1; // this should work
  if (x != 1) abort(); 
  return 0;
}	





