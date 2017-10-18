
/* { dg-do "compile" } */

struct a { int x; };

struct a main()
{
 return 0; /* { dg-error "in return" "" } */
}

