
/* { dg-do "compile" } */
/* { dg-options "-w" } */

int
main ()
{

 enum a { ZERO, ONE, TWO };
 enum b { ZERO };			/* { dg-error "conflicts" } */

}

