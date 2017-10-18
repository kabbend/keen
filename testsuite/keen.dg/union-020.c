
/* { dg-do "run" } */
/* { dg-options "-w" } */

union a { int x[10]; };

int
main ()
{

 union a a_union;
 union a b_union;

 int i;
 for(i=0;i<10;i++) a_union.x[i] = i + 10;

 // union copy
 b_union = a_union;

 printf("b_union.x[3]=%d, .x[9]=%d\n", b_union.x[3], b_union.x[9] );
 if (b_union.x[3] != 13 || b_union.x[9] != 19) abort();

}

