
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main()
{

 typedef union a { int a; int b; } alias;

 alias *x, *y;
 union a *z;

 x = malloc(sizeof(union a));
 y = malloc(sizeof(union a));
 z = malloc(sizeof(union a));

 printf("sizeof union a = %d\n", sizeof(union a));

 (*x).a = 2;
 (*z).b = 5;

 if ((*x).a + (*z).b != 7) abort();
 return 0;

}

