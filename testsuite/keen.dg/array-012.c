
/* { dg-do "compile" } */
/* { dg-options "-w" } */

int
main()
{
 int x[2][4] = { {20,21,22,23,24} , { 100, 101, 102, 103} }; /* { dg-error "too many elements" "" } */
}

