
/* { dg-do "run" } */
/* { dg-options "-w" } */

struct a { int x[10]; int y; char string[12]; };

int
main ()
{

 struct a a_struct;
 struct a b_struct;

 int i;
 for(i=0;i<10;i++) a_struct.x[i] = i + 10;
 a_struct.y = -1;
 strcpy( a_struct.string, "0123456789a" );

 // struct copy
 b_struct = a_struct;

 printf("b_struct.x[3]=%d, .x[9]=%d\n", b_struct.x[3], b_struct.x[9] );
 if (b_struct.x[3] != 13 || b_struct.x[9] != 19) abort();

 printf("b_struct.y = %d\n", b_struct.y );
 if (b_struct.y != -1) abort();

 printf("b_struct.string[]=%s\n",b_struct.string );
 if (strcmp("0123456789a",b_struct.string) != 0) abort();

 return 0;
}

