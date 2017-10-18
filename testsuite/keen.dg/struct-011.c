
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main()
{

 typedef struct a alias;

 alias *x, *y;
 struct a *z;

 struct a { int a, b; };

 x = malloc(8);
 y = malloc(8);
 z = malloc(8);

 (*x).a = 2;
 (*z).b = 5;

 if ((*x).a + (*z).b != 7) abort();
 return 0;

}

