
/* { dg-do "compile" } */

int
main()
{

 typedef union a alias;

 alias x, y; /* { dg-error "unknown size" "" } */
 union a z;

 union a { int a, b; };

 x.a = 2;
 z.b = 5;

 return (x.a + z.b == 7);

}

