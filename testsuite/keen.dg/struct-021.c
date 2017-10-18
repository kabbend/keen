
/* { dg-do "run" } */
/* { dg-options "-w" } */

struct a { int x[10]; };

int
main ()
{

 struct a a_struct;
 struct a b_struct;

 int i;
 for(i=0;i<10;i++) a_struct.x[i] = i + 10;

 // struct copy
 b_struct = a_struct;

 printf("b_struct.x[3]=%d, .x[9]=%d\n", b_struct.x[3], b_struct.x[9] );
 if (b_struct.x[3] != 13 || b_struct.x[9] != 19) abort();

}

