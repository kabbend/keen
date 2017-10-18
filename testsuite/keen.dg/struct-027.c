
/* { dg-do "run" } */
/* { dg-options "-w" } */

struct a { int x; int y; char c[3][7][9]; };

struct a global_ret;

int g(struct a param )
{
 // test copy param for struct
 printf("param.x=%d,param.y=%d\n", param.x, param.y);
 if (param.x != 2 || param.y != 3) abort();
}

int
main ()
{
 struct a z;
 z.x = 2;
 z.y = 3;
 printf("z.x=%d,z.y=%d\n", z.x, z.y);
 g(z);
 
 return 0;
}


