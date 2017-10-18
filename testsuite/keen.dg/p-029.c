/* { dg-do compile } */
/* { dg-options "-w" } */


int
main()
{
  int i = &0x001; 	// { dg-error "" }
}	





