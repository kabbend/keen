
/* { dg-do "compile" } */
/* { dg-options "-w" } */

int
main()
{
 int x[2] = 0; /* { dg-error "cannot initialize" "" } */
}

