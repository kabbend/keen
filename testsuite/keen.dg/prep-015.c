/* { dg-do compile } */
/* { dg-options "-w" } */

#define A( x ,  ... ) 	x __VA_ARGS__ 

int
main()
{
  A (int x, x=0, return x ); // { dg-error "syntax error" }
  return 1;
}
	





