/* { dg-do "run" } */
/* { dg-options "-w" } */

int 
main() 
{

  int x = 2 , y = 3, z = x*y, t = z + 8;
  printf("t=%d\n",t);
  if ( t != 14 ) abort();
  return 0;

}

