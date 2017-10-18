
/* { dg-do "compile" } */
/* { dg-options "-w" } */

int
main ()
{

 enum { ZERO, ONE, TWO };
 // test with a // comment followed by newline 
 
 // another comment. Next line should be 13
 enum b X = 3, Y = -1;		/* { dg-error "incomplete" } */

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

