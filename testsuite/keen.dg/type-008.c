/* { dg-do compile } */
/* { dg-options "-w" } */


int
main()
{
  const int *p;
  int x = 1;
  p = &x;  // allowed
}	





