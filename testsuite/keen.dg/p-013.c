
/* { dg-do "run" } */
/* { dg-options "-w" } */

int x = 3, y;

int
main()
{

 printf("x=%d,y=%d\n",x,y);
 if (x != 3 || y != 0) abort();
 return 0;

}

