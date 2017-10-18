/* { dg-do run } */
/* { dg-options "-w" } */

#define A( x, y, z ) 	x y z 

int
main()
{
  A (return , , 0); 
  return 1;
}
	





