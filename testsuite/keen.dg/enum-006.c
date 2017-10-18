
/* { dg-do "compile" } */
/* { dg-options "-w" } */

int
main ()
{

 enum { ZERO, ONE, TWO };

 enum b { THREE, FOUR, FIVE } X = 3, Y = -1;	
 enum b { SIX } Z = SIX;	/* { dg-error "redefined" } */

 printf("enum b X=%d\n",X);
 printf("enum b Y=%d\n",Y);
 if (X!=3) abort();
 if (Y!=-1) abort();

 printf("ZERO=%d,ONE=%d,TWO=%d\n",ZERO, ONE, TWO);
 printf("THREE=%d,FOUR=%d,FIVE=%d\n",THREE, FOUR, FIVE);

 if (ZERO != 0 || ONE != 1 || TWO != 2) abort();
 if (THREE != 0 || FOUR != 1 || FIVE != 2) abort();

 return 0;
}

