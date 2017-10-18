
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main ()
{

 enum ENUM { ONE = 1, TWO, FOUR = 4, FIVE, SEVEN = 7 };

 int X = FOUR, Y = FIVE;
 int Z = SEVEN;

 printf("X=%d\n",X);
 printf("Y=%d\n",Y);
 printf("Z=%d\n",Z);
 if (X!=4) abort();
 if (Y!=5) abort();
 if (Z!=7) abort();

 return 0;
}

