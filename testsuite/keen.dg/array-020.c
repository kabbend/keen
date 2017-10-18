
/* { dg-do "compile" } */

int
main()
{

 char string[-1] = { 1 }; /* { dg-error "array size" "" } */

}

