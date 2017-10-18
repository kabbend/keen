
/* { dg-do "compile" } */

int
main()
{

 char array1_ok[1][2][3];
 char array_nok[1][][3]; /* { dg-error "empty array size" "" } */

}

