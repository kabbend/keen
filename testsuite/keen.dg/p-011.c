
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main ()
{

 int *y, **z, x;

 x = 2;
 y = &x;
 z = &y;
 printf("%d\n",**z);
 if (**z != 2) abort();
 return 0;

}

