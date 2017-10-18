
/* { dg-do "compile" } */

int
main()
{

 char string[0] = { 1 }; /* { dg-error "array size" "" } */

}

