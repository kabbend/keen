
/* { dg-do "run" } */
/* { dg-options "-w" } */

struct a { int x; int y; char c[3][7][9]; };

struct a global_ret;

struct a g(int x, struct a param, int y, char a_char )
{
 // test copy param for struct
 printf("param.x=%d,param.y=%d\n", param.x, param.y);
 if (param.x != 2 || param.y != 3) abort();
 param.x = y;
 param.y = x;
 param.c[1][2][7] = a_char;
 return param;
}

int
main ()
{
 struct a z;
 z.x = 2;
 z.y = 3;
 printf("z.x=%d,z.y=%d\n", z.x, z.y);
 struct a ret = g(5, z, 9, 22);
 // test return for struct 
 printf("ret.x=%d,ret.y=%d\n", ret.x, ret.y);
 if (ret.x !=  9 || ret.y != 5) abort();
 // test return for struct for a char array 
 printf("ret.c[1][2][7]=%d\n",ret.c[1][2][7]);
 if (ret.c[1][2][7] != 22) abort();
 return 0;
}


