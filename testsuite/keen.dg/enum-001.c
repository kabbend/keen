
/* { dg-do "compile" } */
/* { dg-options "-w" } */

int
main ()
{

 
 enum a { ZERO, ONE, TWO };
 enum a { UN };			/* { dg-error "is redefined" } */
}

