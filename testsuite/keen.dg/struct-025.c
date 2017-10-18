
/* { dg-do "run" } */
/* { dg-options "-w" } */

struct a { int x; int y; };

struct a global_ret;

struct a f(int x, int y)
{
 struct a ret;
 ret.x = x+y;
 ret.y = x+y+1;
 return ret; 
}

int
main ()
{
 struct a z1 = f(3,2);
 printf("z1.x=%d,z1.y=%d\n", z1.x, z1.y);
 global_ret = f(4,2);
 printf("global_ret.x=%d,global_ret.y=%d\n", global_ret.x, global_ret.y);
 if (z1.x + 1 != global_ret.x ) abort();
 if (z1.y + 1 != global_ret.y ) abort();
 return 0;
}

