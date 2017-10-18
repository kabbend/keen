/* { dg-do compile } */
/* { dg-options "-w" } */


const int i = 2;

int
main()
{
  i = 3; 	// { dg-error "" }
}	





