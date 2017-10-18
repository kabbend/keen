
/* { dg-do "run" } */
/* { dg-options "-w" } */

int
main ()
{

 enum { ZERO, ONE, TWO };
 enum b { THREE, FOUR, FIVE };			

 printf("ZERO=%d,ONE=%d,TWO=%d\n",ZERO, ONE, TWO);
 printf("THREE=%d,FOUR=%d,FIVE=%d\n",THREE, FOUR, FIVE);

 if (ZERO != 0 || ONE != 1 || TWO != 2) abort();
 if (THREE != 0 || FOUR != 1 || FIVE != 2) abort();

 return 0;
}

