
/* { dg-do "compile" } */

int
main()
{

 typedef struct a alias;

 alias x, y; /* { dg-error "unknown size" "" } */
 struct a z;

 struct a { int a, b; };

 x.a = 2;
 z.b = 5;

 return (x.a + z.b == 7);

}

