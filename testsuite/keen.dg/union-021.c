
/* { dg-do "run" } */
/* { dg-options "-w" } */

union a { int x[10]; int y; char string[12]; };

int
main ()
{

 union a a_union;
 union a b_union;

 int i;
 for(i=0;i<10;i++) a_union.x[i] = i + 10;
 a_union.y = -1;
 strcpy( a_union.string, "0123456789a" );

 // union copy
 b_union = a_union;

 printf("b_union.string[]=%s\n",b_union.string );
 if (strcmp("0123456789a",b_union.string) != 0) abort();

 return 0;
}

