
/* { dg-do "run" } */
/* { dg-options "-w" } */

union a { int x; int y; char c[3][7][9]; };

union a global_ret;

union a g(int x, union a param, int y, char a_char )
{
 // test copy param for union
 param.c[1][2][7] = a_char;
 return param;
}

int
main ()
{
 union a z;
 union a ret = g(5, z, 9, 22);
 // test return for union for a char array 
 printf("ret.c[1][2][7]=%d\n",ret.c[1][2][7]);
 if (ret.c[1][2][7] != 22) abort();
 return 0;
}


