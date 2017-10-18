/* { dg-do compile } */
/* { dg-options "-w" } */


int
main()
{
  const x = 1;
  volatile y = 2;
  y = x;
}	





